#include "gui.hpp"

#include "control.hpp"

#include <iostream>
#include <algorithm>

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

	button.sprites[button::turn::ON]
		.setTexture(textures[(int)gui::texture_types::BUTTONS]);
	button.sprites[button::turn::ON]
		.setTextureRect(sf::IntRect(0 + step * 25, 25, 25, 25));

	return button;
}

gui::gui(){
	textures.resize((int)gui::texture_types::SIZE);
	textures[(int)gui::texture_types::BUTTONS].loadFromFile("./../data/ui_buttons.png");

	buttons.resize(7);

	buttons[0] = gui::load_button(1);
	buttons[0].upd = [](game_info const& info, button* but){
		if(info.zoom_manager == 3){
			but->state = button::turn::ON;
		} else
			but->state = button::turn::OFF;
	};
	buttons[0].inter = [](game_info &info, button* but){
		change_zoom(&info, 1);
	};

	buttons[1] = gui::load_button(0);
	buttons[1].upd = [](game_info const& info, button* but){
		if(info.zoom_manager == -1){
			but->state = button::turn::ON;
		} else
			but->state = button::turn::OFF;
	};
	buttons[1].inter = [](game_info &info, button* but){
		change_zoom(&info, -1);
	};

	buttons[2] = gui::load_button(6);
	buttons[2].upd = [](game_info const& info, button* but){
		if(info.draw_cells == true){
			but->state = button::turn::ON;
		} else
			but->state = button::turn::OFF;
	};
	buttons[2].inter = [](game_info &info, button* but){
		show_grid(&info);
	};

	buttons[3] = gui::load_button(2);
	buttons[3].upd = [](game_info const& info, button* but){
		if((info.speed == game_info::speed_e::X4) && (info.pause != true)){
			but->state = button::turn::ON;
		} else
			but->state = button::turn::OFF;
	};
	buttons[3].inter = [](game_info &info, button* but){
		set_speed(&info, game_info::speed_e::X4);
	};

	buttons[4] = gui::load_button(3);
	buttons[4].upd = [](game_info const& info, button* but){
		if((info.speed == game_info::speed_e::X2) && (info.pause != true)){
			but->state = button::turn::ON;
		} else
			but->state = button::turn::OFF;
	};
	buttons[4].inter = [](game_info &info, button* but){
		set_speed(&info, game_info::speed_e::X2);
	};

	buttons[5] = gui::load_button(4);
	buttons[5].upd = [](game_info const& info, button* but){
		if((info.speed == game_info::speed_e::X1) && (info.pause != true)){
			but->state = button::turn::ON;
		} else
			but->state = button::turn::OFF;
	};
	buttons[5].inter = [](game_info &info, button* but){
		set_speed(&info, game_info::speed_e::X1);
	};

	buttons[6] = gui::load_button(5);
	buttons[6].upd = [](game_info const& info, button* but){
		if(info.pause == true){
			but->state = button::turn::ON;
		} else
			but->state = button::turn::OFF;
	};
	buttons[6].inter = [](game_info &info, button* but){
		pause(&info);
	};

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
		offset.x = -foffset;
	}

	but->sprites[button::turn::OFF].setPosition(start_point + offset);
	but->sprites[button::turn::ON].setPosition(start_point + offset);
}

}

void gui::update(game_info *info){
	int i = 0;
	std::for_each(buttons.begin(), buttons.begin() + 3, [&i, &info](auto& but){
		but.upd(*info, &but);
		place_button(info, &but, place_position::UP_RIGHT, to_position::DOWN, i++, 100);
		});

	i = 0;
	std::for_each(buttons.begin() + 3, buttons.end(), [&i, &info](auto& but){
		but.upd(*info, &but);
		place_button(info, &but, place_position::UP_RIGHT, to_position::LEFT, i++, 100);
		});
}

namespace{

sf::Color rgb(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255){
	return sf::Color(red, green, blue, alpha);
}

void draw_label(game_info *info, player_info::relationship_type relationship,
	uint32_t cell_index, uint32_t level, float scale, bool is_select){

	float width = 25.0 * scale;
	float height = 13.0 * scale;

	sf::Vector2f offset{(float)-20 * scale, (float)48 * scale};

	sf::RectangleShape label(sf::Vector2f(width, height));
	label.setOrigin(width / 2, height / 2);
	label.setPosition(perspective(info->map[cell_index].pos + offset, &info->view));
	label.setOutlineThickness(1 * scale);

	switch (relationship) {
		case player_info::relationship_type::NEUTRAL :
			label.setFillColor(is_select ? rgb(70, 69, 69) : rgb(161, 161, 161));
			label.setOutlineColor(rgb(29, 29, 29));
		break;
		case player_info::relationship_type::CONTROL :
			label.setFillColor(is_select ? rgb(25, 98, 39) : rgb(45, 64, 49));
			label.setOutlineColor(rgb(9, 20, 11));
		break;
		case player_info::relationship_type::ALLIANCE :
			label.setFillColor(is_select ? rgb(16, 48, 96) : rgb(76, 97, 129));
			label.setOutlineColor(rgb(15, 20, 27));
		break;
		case player_info::relationship_type::ENEMY :
			label.setFillColor(is_select ? rgb(115, 26, 26) : rgb(139, 73, 73));
			label.setOutlineColor(rgb(27, 14, 14));
		break;
	}

	info->window.draw(label);
}

void labels_draw(game_info *info, player_info* player_info){
	float scale = info->view_size.x / info->Width;
	std::vector<bool> *vision_map = get_vision_map(info, player_info->control_players);

	for(uint32_t player_index = 0; player_index < info->players.size(); ++player_index){
		for(auto &curr_unit_ptr : info->players[player_index].units){
			if(vision_map->at(curr_unit_ptr->cell_index)
				&& (on_screen(&info->map[curr_unit_ptr->cell_index], &info->view))){

				bool is_select = false;
				for(auto &unit : info->players[player_info->player].selected_units){
					auto sel_unit = unit.second.lock();
					if(curr_unit_ptr == sel_unit){
						is_select = true;
					}
				}

				draw_label(info, player_info->relationship[player_index],
					curr_unit_ptr->cell_index, 0, scale, is_select);
			}
		}
	}

}

}

void gui::draw(game_info *info, player_info* player_info){
	this->update(info);
	for(auto &but : buttons){
		info->window.draw(but.sprites[but.state]);
	}

	labels_draw(info, player_info);
}

namespace{
	bool is_inside_f(button const& but, sf::Vector2f pos){
		sf::FloatRect rect = but.sprites[but.state].getGlobalBounds();

		if((rect.left <= pos.x) && ((rect.left + rect.width) >= pos.x) &&
			(rect.top <= pos.y) && ((rect.top + rect.height) >= pos.y))
			return true;
		else
			return false;
	}
}

bool gui::gui_interact(game_info *info){
	sf::Vector2f pos = mouse_on_map(info);

	for(auto &but : buttons){
		if(is_inside_f(but, pos)){
			but.inter(*info, &but);
			return true;
		}
	}
	return false;
}

void show_cursor_point(game_info *info){
	sf::Vector2f pos = mouse_on_map(info);

	sf::CircleShape circle(200);
	circle.setRadius(0.4);
	circle.setOrigin(0.2, 0.2);
	circle.setPosition(pos);
	info->window.draw(circle);
}
