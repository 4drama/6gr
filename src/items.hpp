#ifndef ITEMS_HPP
#define ITEMS_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"
#include "deletion_container.hpp"

#include <list>
#include <functional>
#include <map>
#include <string>

struct mech_status;
class mech;
class part_of_mech;

class area;

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

	inline operator bool() const{
		return !(elements.empty() && text_elements.empty() && bar_elements.empty());};
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
class change_mech_status;
class turn_on;
class weapon;
class capacity_change;
class path_draw;

class item_base{
public:
	inline virtual legs* is_legs() noexcept {return nullptr;};
	inline virtual engine* is_engine() noexcept {return nullptr;};
	inline virtual change_mech_status* is_change_mech_status() noexcept {return nullptr;};
	inline virtual turn_on* is_turn_on() noexcept {return nullptr;};
	inline virtual weapon* is_weapon() noexcept {return nullptr;};
	inline virtual capacity_change* is_capacity_change() noexcept {return nullptr;};
	inline virtual path_draw* is_path_draw() noexcept {return nullptr;};

	inline virtual bool get_power_status() const noexcept{ return true;};

	inline virtual item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position){return item_shape{};};
};

class capacity_change : public virtual item_base{
public:
	inline capacity_change* is_capacity_change() noexcept override {return this;};
	virtual mech_status get() const noexcept = 0;
};

class path_draw : public virtual item_base{
public:
	inline path_draw* is_path_draw() noexcept override {return this;};
	inline virtual void draw_active_zone(uint32_t mech_cell_position, game_info *info,
		client *client){return ;};
};

class turn_on : public virtual item_base{
public:
	turn_on(bool power_status);
	inline turn_on* is_turn_on() noexcept override {return this;};

	inline bool get_power_status() const noexcept{ return power_status;};
	inline void power_switch() noexcept{ power_status = power_status ? false : true;};
	inline void power_switch(bool status) noexcept{ this->power_status = status;};

	virtual item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position);
private:
	bool power_status;
protected:
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;
	static void load_turn_on_sprite();
};

class change_mech_status : public virtual item_base{
public:
	inline virtual mech_status get_mech_changes(float time,
		const mech_status &status) const noexcept;
	inline change_mech_status* is_change_mech_status() noexcept override {return this;};
};

class item : public virtual item_base, std::enable_shared_from_this<item>{
public:
	item(const part_of_mech *part_ptr, std::string name, float weight, uint32_t slots);

	inline const std::string& get_name() const noexcept{ return this->name;};
	inline virtual void update(mech* owner, float time){};

	inline float get_weight() const noexcept {return this->weight;};
	inline uint32_t get_slots() const noexcept {return this->slots;};

	bool is_working() const noexcept;
private:
	std::string name;

	float weight = 0;
	uint32_t slots = 0;

	const part_of_mech *part_ptr = nullptr;
};

class torpedo_info;

class weapon : public item, public path_draw{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:

	void use(game_info *info, mech* owner, uint32_t target_cell);

	inline weapon* is_weapon() noexcept override {return this;};

	weapon(deferred_deletion_container<sf::Text> *text_delete_contaier,
		const part_of_mech *part_ptr, std::string name, float delay);
	void update(mech* owner, float time) override;

	bool has_resources(const mech* owner) const noexcept;
	inline float get_delay() const noexcept { return curr_delay / delay;};
	bool get_ready(const mech* owner) const noexcept;

	item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position) override;
	void draw_active_zone(uint32_t mech_cell_position, game_info *info,
		client *client) override;

	std::shared_ptr<torpedo_info>
	torpedo_loading(std::shared_ptr<torpedo_info> torpedo) noexcept;

private:
	std::shared_ptr<sf::Text> text_ptr;

	float shot_energy;
	float shot_heat;

	float delay_energy;

	float curr_delay = 0;
	float delay;

	uint32_t range = 5;
	std::shared_ptr<torpedo_info> torpedo_info_ptr = nullptr;
};

class torpedo_info{
public:
	virtual void create_projectile(game_info *info, mech* owner,
		std::list<uint32_t> path, uint32_t target_cell,
		std::shared_ptr<torpedo_info> torpedo_info_ptr) const = 0;
	virtual area get_damage_zone(game_info *info, uint32_t target_cell) const = 0;
	virtual void detonate(game_info *info, uint32_t target_cell) const = 0;
private:

};

class damage_info{
public:
	enum class part_t{
		TORSO = 0,
		LEFT_ARM = 1,
		RIGHT_ARM = 2,

		SIZE = 3
	};

	struct damage_t{
		float damage;
		float heat;
	};

	struct damage_to_part_t{
		int chance = 100;
		std::pair<float, float> damage;
		std::pair<float, float> heat;
	};

	damage_info(std::map<part_t, damage_to_part_t> damages);
	damage_t get(part_t part) const noexcept;

private:
	std::map<part_t, damage_to_part_t> damages;
};

class explosive_torpedo : public torpedo_info{
public:
	explosive_torpedo();
	void create_projectile(game_info *info, mech* owner,
		std::list<uint32_t> path, uint32_t target_cell,
		std::shared_ptr<torpedo_info> torpedo_info_ptr) const override;
	area get_damage_zone(game_info *info, uint32_t target_cell) const override;
	void detonate(game_info *info, uint32_t target_cell) const override;
private:
	uint32_t aoe = 1;
	std::vector<damage_info> damage;
};

class legs : public item, public turn_on{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	legs(const part_of_mech *part_ptr, std::string name);
	inline legs* is_legs() noexcept override {	return this;};

	inline float get_speed(terrain_en ter_type) const noexcept{
		return this->speed[(int)ter_type] * ((float)2 / 10000)
			* modes[(int)current_mode].rate;};

	mech_status get_mech_changes_legs(float time) const noexcept;
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

class engine : public item, public change_mech_status, public turn_on{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	engine(deferred_deletion_container<sf::Text> *text_delete_contaier,
		const part_of_mech *part_ptr, std::string name, int threshold);
	inline engine* is_engine() noexcept override {return this;};

	mech_status get_mech_changes(float time, const mech_status &status) const noexcept override;

	item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position) override;

	inline float get_threshold() const noexcept{return (float)threshold / 100.0f;};

private:
	struct {
		float receive_energy;
		float receive_heat;
		float spend_fuel;
	} performance;

	int threshold;
	inline void add_threshold(int value) noexcept{
		this->threshold += value;
		this->threshold = this->threshold > 100 ? 100 : this->threshold;
		this->threshold = this->threshold < 0 ? 0 : this->threshold;
	};

	std::shared_ptr<sf::Text> threshold_text_ptr;
	std::shared_ptr<sf::Text> threshold_value_text_ptr;
	
//	mutable sf::Text threshold_text;
//	mutable sf::Text threshold_value_text;
};

class cooling_system : public item, public change_mech_status, public turn_on {
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	cooling_system(const part_of_mech *part_ptr, std::string name, float heat_efficiency, float energy_consumption);
	mech_status get_mech_changes(float time, const mech_status &status) const noexcept override;
	item_shape get_draw_shape(const mech* owner, client *client,
		const sf::Vector2f& position) override;
private:
	float heat_efficiency = 0;
	float energy_consumption = 0;
};

class accumulator : public item, public capacity_change{
public:
	accumulator(const part_of_mech *part_ptr, std::string name, float capacity_);
	mech_status get() const noexcept;
private:
	float capacity = 0;
};

class radiator : public item, public capacity_change{
public:
	radiator(const part_of_mech *part_ptr, std::string name, float capacity_);
	mech_status get() const noexcept;
private:
	float capacity = 0;
};

class tank : public item, public capacity_change{
public:
	tank(const part_of_mech *part_ptr, std::string name, float capacity_);
	mech_status get() const noexcept;
private:
	float capacity = 0;
};

#endif
