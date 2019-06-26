#include "gui.hpp"

#include "control.hpp"

#include <iostream>

gui& gui::instance(){
	static gui i{};
	return i;
}

button gui::load_button(uint32_t step){
	button button{};
	button.sprites[button::turn::OFF]
		.setTexture(textures[(int)gui::texture_types::BUTTONS]);
	button.sprites[button::turn::OFF]
		.setTextureRect(sf::IntRect(0 + step * 25, 0, 25, 25));
	button.sprites[button::turn::OFF]
		.setScale (0.2, -0.2);

	button.sprites[button::turn::ON]
		.setTexture(textures[(int)gui::texture_types::BUTTONS]);
	button.sprites[button::turn::ON]
		.setTextureRect(sf::IntRect(0 + step * 25, 25, 25, 25));
	button.sprites[button::turn::OFF]
		.setScale (0.2, -0.2);

	return button;
}

gui::gui(){
	textures.resize((int)gui::texture_types::SIZE);
	textures[(int)gui::texture_types::BUTTONS].loadFromFile("./../data/ui_buttons.png");

	buttons.resize(2);
	buttons[0] = gui::load_button(0);
	buttons[1] = gui::load_button(1);
}

namespace{
enum class place_position{
	UP_RIGHT
};

enum class to_position{
	DOWN,
	LEFT
};

void place_button(game_info *info, button *but, place_position place, to_position to,
	uint32_t step, float start_offset){
	float scale = info->view_size.x / info->Width;

	but->sprites[button::turn::OFF].setScale (scale, -scale);
	but->sprites[button::turn::ON].setScale (scale, -scale);

	const float button_size = 25;

	sf::Vector2f start_point;
	if(place == place_position::UP_RIGHT)
		start_point = sf::Vector2f(info->view_size.x / 2 - (button_size + 2) * scale,
			-info->view_size.y / 2 - 4 * scale);

	float foffset = (start_offset + step * (25 + 2)) * scale;
	sf::Vector2f offset{0, 0};
	if(to == to_position::DOWN){
		offset.y = -foffset;
	} else if(to == to_position::LEFT){
		offset.x = foffset;
	}

	but->sprites[button::turn::OFF].setPosition(start_point + offset);
	but->sprites[button::turn::ON].setPosition(start_point + offset);
}

}

void gui::update(game_info *info){
	float scale = info->view_size.x / info->Width;

	int i = 0;
	for(auto &but : buttons){
		place_button(info, &but, place_position::UP_RIGHT, to_position::DOWN, i++, 100);
	}
}

void gui::draw(game_info *info){
	this->update(info);
	for(auto &but : buttons){
		info->window.draw(but.sprites[0]);
	}
}
