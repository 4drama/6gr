#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SFML/Graphics.hpp>
#include <list>

class game_window{
public:
	game_window(sf::Vector2f position, sf::Vector2f size, sf::Color color);
	void draw(sf::RenderWindow *window, float scale);
	bool interact(sf::Vector2f pos, sf::Event event);

	inline bool is_close() const noexcept {return this->is_close_m;};
private:
	sf::Vector2f position;
	sf::Vector2f size;
	float scale;
	sf::Vector2f offset{0, 0};
	
	std::list<sf::RectangleShape> shape;

	bool is_move = false;
	sf::Vector2f old_move_position;
	bool is_close_m = false;
};

#endif
