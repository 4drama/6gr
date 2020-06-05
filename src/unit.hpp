#ifndef UNIT_HPP
#define UNIT_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

#include <map>
#include <list>
#include <memory>

struct unit : std::enable_shared_from_this<unit>{
	static std::map<std::string, sf::Texture> textures;

	uint32_t cell_index;
	std::vector<sf::Sprite> sprites;

	uint32_t vision_range = 0;
	std::vector<uint32_t> vision_indeces{};

	float speed[(int)terrain_en::END];
	float speed_mod = (float)2 / 10000;
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

	inline float get_speed(terrain_en ter_type){return this->speed[(int)ter_type];};

private:
	void unit_update_move(game_info *info, uint32_t player_index, float time);
};

class mech : public unit{
public:
	static inline std::shared_ptr<mech> create(uint32_t cell_index){
		return std::make_shared<mech>(cell_index);};

	mech(uint32_t cell_index);
private:
};

#endif
