#ifndef UNIT_HPP
#define UNIT_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

#include <map>
#include <list>
#include <memory>
#include <string>
#include <utility>

bool is_inside_sprite(sf::Sprite sprite, sf::Vector2f pos);

struct item_shape{
	std::list<sf::Sprite> elements;
	std::list<sf::Text*> text_elements;

	std::list<sf::RectangleShape> bar_elements;
};

class item{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
/*	static inline const sf::Sprite& get_sprite(std::string name) const noexcept{
		return sprites[name]};*/
public:
	item(std::string name, float delay);

	inline const bool& get_power_status() const noexcept{ return power_status;};
	inline void power_switch() noexcept{ power_status = power_status ? false : true;};
	inline void power_switch(bool status) noexcept{ this->power_status = status;};

	inline const std::string& get_name() const noexcept{ return this->name;};

	inline const sf::Keyboard::Key& get_hotkey() const noexcept { return this->key;};
	inline void set_hotkey(sf::Keyboard::Key key_) noexcept { this->key = key_;};

	inline float get_delay() const noexcept { return curr_delay / delay;};
	bool get_ready() const noexcept;

	inline bool has_ammo() const noexcept { return true;};

	void update(float time);

	item_shape get_draw_shape(client *client, const sf::Vector2f& position) const;
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

struct unit : std::enable_shared_from_this<unit>{
	static std::map<std::string, sf::Texture> textures;

	uint32_t cell_index;
	std::vector<sf::Sprite> sprites;

	uint32_t vision_range = 0;
	std::vector<uint32_t> vision_indeces{};

	std::list<uint32_t> path;
	float path_progress = 0;

/*	enum class weight_level_type{
		LIGHT,
		LOADED,
		OVERLOADED
	};
	static std::shared_ptr<unit> create_caravan(weight_level_type weight, uint32_t cell_index);
*/

	unit(uint32_t cell_index, uint32_t vision_range);

	void open_vision(game_info *info, uint32_t player_index);
	void update(game_info *info, uint32_t player_index, float time);

	inline virtual float get_speed(terrain_en ter_type) const noexcept {return 0;};
	virtual void draw_gui(game_info *info, client *client){return ;};
	virtual bool interact_gui(game_info *info, client *client){return false;};

private:
	void unit_update_move(game_info *info, uint32_t player_index, float time);
	inline virtual void update_v(game_info *info, uint32_t player_index, float time){return ;};
};

class mech : public unit{
public:
	static inline std::shared_ptr<mech> create(uint32_t cell_index){
		return std::make_shared<mech>(cell_index);};

	mech(uint32_t cell_index);

	inline float get_speed(terrain_en ter_type) const noexcept override
		{return this->speed[(int)ter_type] * ((float)2 / 10000);};

	void draw_gui(game_info *info, client *client);
	bool interact_gui(game_info *info, client *client);
private:
	item *waiting_confirm = nullptr;

	float current_energy = 75;
	float energy_capacity = 100;
	mutable sf::Text energy_text;

	float current_heat = 0;
	float heat_capacity = 300;

	std::list<item> left_arm;
	std::list<item> torso;
	std::list<item> right_arm;

	float speed[(int)terrain_en::END];
	item weapon;

	item_shape get_status_shape(client *client, const sf::Vector2f& position) const;
	void update_v(game_info *info, uint32_t player_index, float time);
};

#endif
