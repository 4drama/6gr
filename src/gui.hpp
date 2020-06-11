#ifndef GUI_HPP
#define GUI_HPP

#include <SFML/Graphics.hpp>

struct button;
class gui;

#include "map.hpp"
#include "client.hpp"

#include <functional>

struct button{
	enum turn{
		OFF = 0,
		ON = 1
	};
	sf::Sprite sprites[2];
	turn state;

	std::function<void(game_info const& info, client const& client, button* but)> upd;
	std::function<void(game_info &info, client &client, button* but)> inter;
};

class gui{
public:
	gui(gui const &) = delete;
	gui(gui &&) = delete;

	gui& operator= (gui const&) = delete;

	static gui& instance();

	void draw(game_info *info, client *client);
	bool gui_interact(game_info *info, client *client);

private:
	gui();
	~gui() = default;

	enum class texture_types{
		BUTTONS = 0,
		SIZE = 1
	};
	std::vector< sf::Texture > textures{};
	std::vector< button > buttons{};
	button load_button(uint32_t step);

	void update(game_info *info, client *client);
};

inline const sf::Font& get_font(){
	static sf::Font *font = nullptr;
	if(!font){
		font = new sf::Font{};
		font->loadFromFile("./../data/times_new_roman.ttf");
	}
	return *font;
}

#endif
