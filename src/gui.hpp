#ifndef GUI_HPP
#define GUI_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

struct button{
	enum turn{
		OFF = 0,
		ON = 1
	};
	sf::Sprite sprites[2];
};

class gui{
public:
	gui(gui const &) = delete;
	gui(gui &&) = delete;

	gui& operator= (gui const&) = delete;

	static gui& instance();

	void update(game_info *info);
	void draw(game_info *info);

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
};

#endif
