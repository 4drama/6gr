#include "control.hpp"

#include "gui.hpp"

#include <iostream>

void change_zoom(game_info *info, int value){
	if((value < 0) && (info->zoom_manager > -1)){
		while(value++){
			--info->zoom_manager;
			info->view_size.x += 50;
			info->view_size.y += 50 / info->display_rate;
		}
	} else if((value > 0) && (info->zoom_manager < 3)){
		while(value--){
			++info->zoom_manager;
			info->view_size.x -= 50;
			info->view_size.y -= 50 / info->display_rate;
		}
	}

	info->view.setSize(info->view_size);
}

void pause(game_info *info){
	info->pause = !info->pause;
}

void speed_up(game_info *info){
	if((info->pause != true) && (info->speed != game_info::speed_e::X4))
		info->speed = (game_info::speed_e)((int)info->speed + 1);
}

void speed_down(game_info *info){
	if((info->pause != true) && (info->speed != game_info::speed_e::X1))
		info->speed = (game_info::speed_e)((int)info->speed - 1);
}

void show_grid(game_info *info){
	info->draw_cells = !info->draw_cells;
}

void set_speed(game_info *info, game_info::speed_e speed){
	info->pause = false;
	info->speed = speed;
}

void select_cell(game_info *info, uint32_t player_index){
	info->players[player_index].selected_cell = get_cell_index_under_mouse(info);
	info->players[player_index].selected_units.clear();
}

void select_units(game_info *info, uint32_t player_index){
	std::list<uint32_t> players{};
	players.push_back(player_index);

	std::list<uint32_t> cells{};
	cells.push_back(info->players[player_index].selected_cell);

	auto units = units_on_cells(info, players, cells);

	info->players[player_index].selected_units.clear();
	for(auto &curr_unit : units){
		info->players[player_index].selected_units.emplace_back(curr_unit);
	}
}

void event_handler(game_info *info, float time, uint32_t player_index){
	sf::Event event;
	float speed = 5;

	while(info->window.pollEvent(event)){
		if(event.type == sf::Event::Closed ||
		  (event.type == sf::Event::KeyPressed &&
		   event.key.code == sf::Keyboard::Escape))
			info->window.close();


		static sf::Vector2f last_pasition = sf::Vector2f(0, 0);
		static bool is_map_move = false;
		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Button::Middle){

			is_map_move = true;
			last_pasition = mouse_on_map(info);

		} else if(event.type == sf::Event::MouseMoved ){

			if(is_map_move){
				sf::Vector2f diff = last_pasition - mouse_on_map(info);
				last_pasition = mouse_on_map(info);

				move_map(&info->map, cardinal_directions_t::WEST, diff.x * 0.65);
				move_map(&info->map, cardinal_directions_t::SOUTH, diff.y * 1.1);
			}
		}
		if (event.type == sf::Event::MouseButtonReleased &&
			event.mouseButton.button == sf::Mouse::Button::Middle){
			is_map_move = false;
		}

		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Left){
			move_map(&info->map, cardinal_directions_t::EAST, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Right){
			move_map(&info->map, cardinal_directions_t::WEST, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Up){
			move_map(&info->map, cardinal_directions_t::SOUTH, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Down){
			move_map(&info->map, cardinal_directions_t::NORTH, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Subtract){
			speed_down(info);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Add){
			speed_up(info);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Space){
			pause(info);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::LBracket){
			change_zoom(info, -1);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::RBracket){
			change_zoom(info, 1);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::F1){
			show_grid(info);
		}

		static float left_click_cd = 0;
		left_click_cd -= time;
		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Button::Left){
			if(left_click_cd <= 0){
				if(!gui::instance().gui_interact(info)){
		  			select_cell(info, player_index);
					select_units(info, player_index);
				}
				left_click_cd = 30;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Button::Right){

			if(info->players[player_index].selected_units.size() != 0){
				auto *selected_units = &info->players[player_index].selected_units;
				auto finish_cell = get_cell_index_under_mouse(info);

				for(auto &curr_unit : *selected_units){
					if((curr_unit.first == player_index) && (curr_unit.second.use_count() != 0)){
						std::shared_ptr<unit> unit_ptr = curr_unit.second.lock();

						unit_ptr->path = path_find(info,
							unit_ptr->cell_index, finish_cell, unit_ptr, player_index);
						unit_ptr->path_progress = 0;
					}
				}
			}
		}

		if( event.type == sf::Event::MouseWheelScrolled ){
			change_zoom(info, event.mouseWheelScroll.delta);
		}

	}
}
