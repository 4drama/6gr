#include "control.hpp"

void event_handler(game_info *info, float time, uint32_t player_index){
	sf::Event event;
	float speed = 5;

	while(info->window.pollEvent(event)){
		if(event.type == sf::Event::Closed ||
		  (event.type == sf::Event::KeyPressed &&
		   event.key.code == sf::Keyboard::Escape))
			info->window.close();

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

			if(info->zoom_manager > -1){
				--info->zoom_manager;
				info->view_size.x += 50;
				info->view_size.y += 50 / info->display_rate;
				info->view.setSize(info->view_size);
			}
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Add){
			if(info->zoom_manager < 3){
				++info->zoom_manager;
				info->view_size.x -= 50;
				info->view_size.y -= 50 / info->display_rate;
				info->view.setSize(info->view_size);
			}
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::F1){
			if(info->draw_cells)
				info->draw_cells = false;
			else
				info->draw_cells = true;
		}

		static float left_click_cd = 0;
		left_click_cd -= time;
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
			if(left_click_cd <= 0){
		  		select_cell(info, player_index);
				left_click_cd = 30;
			}
		}
	}
}
