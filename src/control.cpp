#include "control.hpp"

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
		  		select_cell(info, player_index);
				left_click_cd = 30;
			}
		}

		if( event.type == sf::Event::MouseWheelScrolled ){
			change_zoom(info, event.mouseWheelScroll.delta);
		}

	}
}
