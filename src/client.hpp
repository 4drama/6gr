#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <SFML/Graphics.hpp>

class client;

#include "map.hpp"

class client{
public:
	client(game_info *info, int width_, int height_, uint32_t player_index);

	float get_view_scale() const;

	sf::Vector2f perspective(sf::Vector2f position) const;
	bool on_screen(cell *cell) const;
private:
	int Width, Height;

	sf::RenderWindow window;
	sf::View view;

	sf::Vector2f view_size;
	float display_rate;

	bool draw_cells;
	int zoom_manager;

	std::list<player_info> control_players;
};

#endif
