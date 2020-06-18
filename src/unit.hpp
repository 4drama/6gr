#ifndef UNIT_HPP
#define UNIT_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

#include <map>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <functional>

bool is_inside_sprite(sf::Sprite sprite, sf::Vector2f pos);

struct mech_status;
class mech;

struct item_button{
	sf::Sprite sprite;
	std::function<void()> func;

	item_button() = default;
	item_button(sf::Sprite sprite_, std::function<void()> func_);
};

struct item_shape{
	std::list<item_button> elements;
	std::list<sf::Text*> text_elements;

	std::list<sf::RectangleShape> bar_elements;

	inline item_shape& operator=(const item_shape& right){
		if (this == &right) {
            return *this;
        }
		this->elements = right.elements;
		this->text_elements = right.text_elements;
		this->bar_elements = right.bar_elements;

		return *this;
	}
};

inline const item_shape operator+(const item_shape& left, const item_shape& right){
	item_shape shape(left);
	auto copy_to = [](auto &from, auto &to){
		for(auto &el : from){
			to.emplace_back(el);
		}
	};

	copy_to(right.elements, shape.elements);
	copy_to(right.text_elements, shape.text_elements);
	copy_to(right.bar_elements, shape.bar_elements);

    return shape;
}

inline item_shape& operator+=(item_shape& left, const item_shape& right){
	left = left + right;
    return left;
}

class legs;
class engine;

class item : std::enable_shared_from_this<item>{
protected:
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	inline virtual legs* is_legs() noexcept {return nullptr;};
	inline virtual engine* is_engine() noexcept {return nullptr;};

	item(std::string name, float delay);

	inline const bool& get_power_status() const noexcept{ return power_status;};
	inline void power_switch() noexcept{ power_status = power_status ? false : true;};
	inline void power_switch(bool status) noexcept{ this->power_status = status;};

	inline const std::string& get_name() const noexcept{ return this->name;};

	inline const sf::Keyboard::Key& get_hotkey() const noexcept { return this->key;};
	inline void set_hotkey(sf::Keyboard::Key key_) noexcept { this->key = key_;};

	inline float get_delay() const noexcept { return curr_delay / delay;};
	bool get_ready(const mech* owner) const noexcept;

	inline virtual bool has_resources(const mech* owner) const noexcept { return true;};

	virtual void update(mech* owner, float time);

	virtual item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position);
private:
	std::string name;
	sf::Keyboard::Key key = sf::Keyboard::Unknown;
	mutable sf::Text name_text;

	float curr_delay = 0;
	float delay;

	bool power_status = true;
};

class legs : public item{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	legs(std::string name);
	inline legs* is_legs() noexcept override {	return this;};
//	void update(mech* owner, float time);

	inline float get_speed(terrain_en ter_type) const noexcept{
		return this->speed[(int)ter_type] * ((float)2 / 10000)
			* modes[(int)current_mode].rate;};

	inline mech_status necessary(float time) const noexcept;

	item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position) override;
private:
	float speed[(int)terrain_en::END];

	enum class mode_name{
		slow = 0,
		medium = 1,
		fast = 2,
		size = 3
	};
	mode_name current_mode = mode_name::medium;
	struct mode{
		float rate;
		float energy;
		float heat;
	};
	mode modes[(int)mode_name::size];

	inline void set_mode(mode_name mode) noexcept {this->current_mode = mode;};
};

class engine : public item{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	engine(std::string name, int threshold);
	inline engine* is_engine() noexcept override {return this;};

	item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position) override;

	inline float get_threshold() const noexcept{return (float)threshold / 100.0f;};

private:
	inline void add_threshold(int value) noexcept{
		this->threshold += value;
		this->threshold = this->threshold > 100 ? 100 : this->threshold;
		this->threshold = this->threshold < 0 ? 0 : this->threshold;
	};

	int threshold;
	mutable sf::Text threshold_text;
	mutable sf::Text threshold_value_text;
};

struct unit : std::enable_shared_from_this<unit>{
	static std::map<std::string, sf::Texture> textures;

	uint32_t cell_index;
	std::vector<sf::Sprite> sprites;

	uint32_t vision_range = 0;
	std::vector<uint32_t> vision_indeces{};

	std::list<uint32_t> path;
	float path_progress = 0;

	unit(uint32_t cell_index, uint32_t vision_range);

	void open_vision(game_info *info, uint32_t player_index);
	void update(game_info *info, uint32_t player_index, float time);

	inline virtual float get_speed(terrain_en ter_type) const noexcept {return 0;};
	virtual void draw_gui(game_info *info, client *client){return ;};
	virtual bool interact_gui(game_info *info, client *client){return false;};

private:
	void unit_update_move(game_info *info, uint32_t player_index, float time);
	inline virtual void update_v(game_info *info, uint32_t player_index, float time){return ;};
	virtual float move_calculate(float time, terrain_en ter_type) noexcept{
		return this->get_speed(ter_type) * time;};
};

struct mech_status {
	float current_energy = 75;
	float energy_capacity = 100;

	float current_heat = 50;
	float heat_capacity = 300;

	float current_fuel = 30;
	float fuel_capacity = 100;

	inline static mech_status capacity(float energy, float heat, float fuel = 0){
		return mech_status{0, energy, 0, heat, 0, fuel};};

	inline static mech_status current(float energy, float heat, float fuel = 0){
		return mech_status{energy, 0, heat, 0, fuel, 0};};

	inline mech_status& operator*(float right);
	inline mech_status& add_capacity(const mech_status& right);
	inline mech_status& add_current(const mech_status& right);
};

class mech : public unit{
public:
	static inline std::shared_ptr<mech> create(uint32_t cell_index){
		return std::make_shared<mech>(cell_index);};

	mech(uint32_t cell_index);

	inline float get_speed(terrain_en ter_type) const noexcept override
		{return legs_ptr ? legs_ptr->get_speed(ter_type) : 0;};

	void draw_gui(game_info *info, client *client);
	bool interact_gui(game_info *info, client *client);
private:
	item *waiting_confirm = nullptr;
	mech_status status;

	mutable sf::Text energy_text;
	mutable sf::Text heat_text;
	mutable sf::Text fuel_text;

	legs *legs_ptr = nullptr;
	engine *engine_ptr = nullptr;

	std::list<std::shared_ptr<item>> left_arm;
	std::list<std::shared_ptr<item>> torso;
	std::list<std::shared_ptr<item>> right_arm;

	item_shape get_status_shape(client *client, const sf::Vector2f& position) const;
	void update_v(game_info *info, uint32_t player_index, float time);

	item_shape prepare_shape(client *client) const;
	void refresh();

	float move_calculate(float time, terrain_en ter_type) noexcept override;
};

inline mech_status& mech_status::operator*(float right){
	this->current_energy *= right;
	this->current_heat *= right;
	return *this;
}

inline mech_status& mech_status::add_capacity(const mech_status& right){
	this->energy_capacity += right.energy_capacity;
	this->heat_capacity += right.heat_capacity;
	return *this;
};

inline mech_status& mech_status::add_current(const mech_status& right){
	this->current_energy += right.current_energy;
	this->current_heat += right.current_heat;
	return *this;
};

inline mech_status legs::necessary(float time) const noexcept{
	const float& energy = this->modes[(int)current_mode].energy;
	const float& heat = this->modes[(int)current_mode].heat;
	return mech_status::current(energy, heat) * time;
};

#endif
