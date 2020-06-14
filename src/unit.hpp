#ifndef UNIT_HPP
#define UNIT_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

#include <map>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include <iostream>

bool is_inside_sprite(sf::Sprite sprite, sf::Vector2f pos);

class mech;

struct item_shape{
	std::list<sf::Sprite> elements;
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

class item : std::enable_shared_from_this<item>{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
/*	static inline const sf::Sprite& get_sprite(std::string name) const noexcept{
		return sprites[name]};*/
public:
	inline virtual legs* is_legs() noexcept {return nullptr;};

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
		const sf::Vector2f& position) const;
/*	void draw_button(game_info *info, client *client, sf::Vector2f position) const noexcept;
	void push_button(game_info *info, client *client,
		sf::Vector2f but_position, sf::Vector2f click_position) const noexcept;*/
private:
	std::string name;
	sf::Keyboard::Key key = sf::Keyboard::Unknown;
	mutable sf::Text name_text;

	float curr_delay = 0;
	float delay;

	bool power_status = true;
};

class legs : public item{
public:
	legs(std::string name);
	legs* is_legs() noexcept override {	return this;};
//	void update(mech* owner, float time);

	inline float get_speed(terrain_en ter_type) const noexcept{
		return this->speed[(int)ter_type] * ((float)2 / 10000)
			* modes[(int)current_mode].rate;};

	inline float energy_necessary(float time) const noexcept{
		return time * modes[(int)current_mode].energy;};
private:
	float speed[(int)terrain_en::END];

	enum class mode_name{
		fast = 0,
		medium = 1,
		slow = 2,
		size = 3
	};
	mode_name current_mode = mode_name::medium;
	struct mode{
		float rate;
		float energy;
	};
	mode modes[(int)mode_name::size];
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

	float current_energy = 75;
	float energy_capacity = 100;
	mutable sf::Text energy_text;

	float current_heat = 50;
	float heat_capacity = 300;
	mutable sf::Text heat_text;

	legs *legs_ptr = nullptr;
	std::list<std::shared_ptr<item>> left_arm;
	std::list<std::shared_ptr<item>> torso;
	std::list<std::shared_ptr<item>> right_arm;

//	float speed[(int)terrain_en::END];

	item_shape get_status_shape(client *client, const sf::Vector2f& position) const;
	void update_v(game_info *info, uint32_t player_index, float time);

	item_shape prepare_shape(client *client) const;
	void refresh();

	float move_calculate(float time, terrain_en ter_type) noexcept override;
};

#endif
