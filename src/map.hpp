#ifndef MAP_HPP
#define MAP_HPP

#include <SFML/Graphics.hpp>

#include <vector>
#include <cstdint>
#include <utility>
#include <list>
#include <memory>

struct game_info;

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
		SCHEME = 0,
		PALM = 1,
		MOUNTAIN = 2,

		SIZE = 3
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

	std::vector<bool> player_visible;
};

struct unit : std::enable_shared_from_this<unit>{
	enum class unit_type{
		CARAVAN = 0,

		SIZE = 1
	};
	static std::vector<sf::Texture> textures;
	static void fill_textures();

	uint32_t cell_index;
	std::vector<sf::Sprite> sprites;

	uint32_t vision_range = 0;
	std::vector<uint32_t> vision_indeces{};

	float speed[(int)terrain_en::END];
	float speed_mod = 0;
	std::list<uint32_t> path;
	float path_progress = 0;

	enum class weight_level_type{
		LIGHT,
		LOADED,
		OVERLOADED
	};
	static std::shared_ptr<unit> create_caravan(weight_level_type weight, uint32_t cell_index);

	void open_vision(game_info *info, uint32_t player_index);
	void update(game_info *info, uint32_t player_index, float time);

private:
	void unit_update_move(game_info *info, uint32_t player_index, float time);
};

struct player{
	using player_index_type = uint32_t;
	using selected_unit_type = std::pair< player_index_type, std::weak_ptr<unit> >;
	std::string name = "default";

	std::vector< std::shared_ptr<unit> > units;

	uint32_t selected_cell = UINT32_MAX;
	std::vector< selected_unit_type > selected_units{};
};

struct game_info{
	int Width, Height;

	sf::RenderWindow window;
	sf::View view;

	sf::Vector2f view_size;
	float display_rate;

	std::vector<cell> map;

	std::vector<player> players;
	std::vector<uint32_t> visible_players_indeces;

	bool draw_cells;
	enum speed_e{
		X1,
		X2,
		X4
	} speed;
	bool pause;
	int zoom_manager;

	game_info();

	cell& get_cell(uint32_t index);
	void update(float time);
};

struct player_info{
	uint32_t player;

	std::list<uint32_t> control_players;
	std::list<uint32_t> alliance_players;
	std::list<uint32_t> enemy_players;

	enum relationship_type{
		NEUTRAL = 0,
		CONTROL = 1,
		ALLIANCE = 2,
		ENEMY = 3
	};

	std::vector<relationship_type> relationship;

	player_info(game_info *info, uint32_t player_);
	void update(game_info *info);
};

std::vector<cell> generate_world(uint32_t size);
void generate_level(std::vector<cell> *map);
void draw_map(game_info *info, float time, uint32_t player_index);
void move_map(std::vector<cell> *map, cardinal_directions_t dir, float speed);

uint32_t add_player(game_info *info, std::string name, bool is_visible);
void add_unit(game_info *info, uint32_t player_index /*, unit*/);
void player_respawn(game_info *info, uint32_t player_index);

sf::Vector2f mouse_on_map(game_info *info);

std::list<uint32_t> path_find(game_info *info, uint32_t start_point,
	uint32_t finish_point, std::shared_ptr<unit> unit, uint32_t player_index);

uint32_t get_cell_index_under_mouse(game_info *info);
void draw_path(game_info *info, std::list<uint32_t> path, float progress);

std::list<player::selected_unit_type> units_on_cells(
	game_info *info, std::list<uint32_t> players_indeces, std::list<uint32_t> cells);

void units_draw_paths(game_info *info, uint32_t players_index);

std::vector<bool>* get_vision_map(game_info *info, std::list<uint32_t> players_indeces);

sf::Vector2f perspective(sf::Vector2f position, sf::View *view);
bool on_screen(cell *cell, sf::View *view);

#endif
