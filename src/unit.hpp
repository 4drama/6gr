#ifndef UNIT_HPP
#define UNIT_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

#include <map>
#include <list>
#include <memory>
#include <string>

class item{
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

	inline bool has_ammo() const noexcept {return true;};

	void update(float time);
private:
	std::string name;
	sf::Keyboard::Key key = sf::Keyboard::Unknown;

	float curr_delay = 0;
	float delay;

	bool power_status = false;
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

private:
	void unit_update_move(game_info *info, uint32_t player_index, float time);
};

class mech : public unit{
public:
	static inline std::shared_ptr<mech> create(uint32_t cell_index){
		return std::make_shared<mech>(cell_index);};

	mech(uint32_t cell_index);

	inline float get_speed(terrain_en ter_type) const noexcept override
		{return this->speed[(int)ter_type] * ((float)2 / 10000);};
private:
	float speed[(int)terrain_en::END];
};

#endif
