#include <SFML/Graphics.hpp>

#include <cmath>
#include <vector>
#include <cstdlib>

#include "map.hpp"
#include "control.hpp"
#include "gui.hpp"

#include <iostream>

int main(){
	sf::Clock clock;
	float time;

	game_info info{};
	uint32_t player_index = add_player(&info, "Player 1", true);

	client client1(&info, 1400, 900, player_index);

//	player_info player_info(&info, player_index);

	player_respawn(&info, player_index);

	while(info.window.isOpen()){
		event_handler(&info, time, player_index);

		info.window.clear();
		time = clock.getElapsedTime().asMilliseconds();
		clock.restart();

		info.update(time);

		info.window.setView(info.view);
		draw_map(&info, time, player_index);
		gui::instance().draw(&info, &player_info);

	//	show_cursor_point(&info);

		units_draw_paths(&info, player_index);

		info.window.display();
	}

	return 0;
}
