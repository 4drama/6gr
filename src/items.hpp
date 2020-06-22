#ifndef ITEMS_HPP
#define ITEMS_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

#include <list>
#include <functional>
#include <map>
#include <string>

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
class change_mech_status;

class item_base{
public:
	inline virtual legs* is_legs() noexcept {return nullptr;};
	inline virtual engine* is_engine() noexcept {return nullptr;};
	inline virtual change_mech_status* is_change_mech_status() noexcept {return nullptr;};
};

class change_mech_status : public virtual item_base{
public:
	inline virtual mech_status get_mech_changes(float time,
		const mech_status &status) const noexcept;
	inline change_mech_status* is_change_mech_status() noexcept override {return this;};
};

class item : public virtual item_base, std::enable_shared_from_this<item>{
protected:
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
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

class engine : public item, public change_mech_status{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	engine(std::string name, int threshold);
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

	mutable sf::Text threshold_text;
	mutable sf::Text threshold_value_text;
};

#endif
