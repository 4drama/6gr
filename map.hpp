#ifndef MAP_HPP
#define MAP_HPP

#include <SFML/Graphics.hpp>

#include <vector>
#include <cstdint>
#include <utility>

enum class cardinal_directions_t{
	BEGIN = 0,

	WEST = 0,
	WEST_NORTH = 1,
	EAST_NORTH = 2,
	EAST = 3,
	EAST_SOUTH = 4,
	WEST_SOUTH = 5,

	LAST = 5,
	END = 6,

	NORTH = 6,
	SOUTH = 7
};

enum class terrain_en{
	BEGIN = 0,

	RIVER = 0,
	MOUNTAIN = 1,
	PLAIN = 2,
	PALM = 3,

	LAST = 3,
	END = LAST + 1
};



struct object{
	enum class texture_type{
		PALM = 0,
		MOUNTAIN = 1,

		SIZE = 2
	};

	static std::vector<sf::Texture> textures;
	static void fill_textures();

	std::vector<std::pair<float, sf::Sprite> > sprites;
	float time_point = 0;
	float last_time_point;

	sf::Vector2f pos{0, 0};
	float size;

	static object create_palm(std::vector<object> *objects);
	static object create_mountain(std::vector<object> *objects);
	sf::Sprite update_sprite(float time);
};

struct terrain{
	static uint32_t gen_rate[(int)terrain_en::END][(int)terrain_en::END];
	static void fill_gen_rate();

	terrain_en type;
	std::vector<object> objects;
};

cardinal_directions_t against(cardinal_directions_t dir);
cardinal_directions_t next(cardinal_directions_t dir);
cardinal_directions_t previous(cardinal_directions_t dir);

class cell{
public:
	static sf::Vector2f offset[(int)cardinal_directions_t::END];
	static float side_size;
	static sf::Vertex shape[7];

	static void set_side_size(float side_size_);

	terrain ter = terrain{terrain_en::PLAIN};
	sf::Vector2f pos = sf::Vector2f{0, 0};
	uint32_t indeces[(int)cardinal_directions_t::END] =
		{ UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

struct player{

};

struct game_info{
	int Width, Height;

	sf::RenderWindow window;
	sf::View view;

	int zoom_manager;
	sf::Vector2f view_size;
	float display_rate;

	std::vector<cell> map;
	bool draw_cells;

	std::vector<player> players;

	game_info();
};

std::vector<cell> generate_world(uint32_t size);
void generate_level(std::vector<cell> *map);
void draw_map(game_info *info, float time);
void move_map(std::vector<cell> *map, cardinal_directions_t dir, float speed);

#endif
