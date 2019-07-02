#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <SFML/Graphics.hpp>

struct player_info;
class client;

#include "map.hpp"

#include <list>
#include <functional>

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

class client{
public:
	client(game_info *info, int width_, int height_, uint32_t player_index);

	float get_view_scale() const;

	sf::Vector2f perspective(sf::Vector2f position) const;
	bool on_screen(cell *cell) const;
	bool is_visable(cell *cell) const;

	void draw_cell(sf::Vertex *transform_shape, cell *cell,
		std::function<sf::Color(terrain_en)> func) const;
	void draw_out_of_view(cell *cell) const;
	void draw_objects(std::vector<sf::Sprite> *object_sprites) const;

	void draw_cell_border(sf::Vertex *transform_shape) const;
	std::vector<uint32_t> get_vision_indeces(game_info *info) const;
	sf::Vector2f mouse_on_map() const;
private:
	int Width, Height;

	sf::RenderWindow window;
	sf::View view;

	sf::Vector2f view_size;
	float display_rate;

	bool draw_cells;
	int zoom_manager;

	player_info player;

};

#endif
