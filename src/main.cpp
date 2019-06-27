#include <SFML/Graphics.hpp>

#include <cmath>
#include <vector>
#include <cstdlib>

#include "map.hpp"
#include "control.hpp"
#include "gui.hpp"

#include <iostream>

int main(){
	game_info info{};
	sf::Clock clock;
	float time;

	uint32_t player_index = add_player(&info, "Player 1", true);
	player_respawn(&info, player_index);

	while(info.window.isOpen()){
		event_handler(&info, time, player_index);

		info.window.clear();
		time = clock.getElapsedTime().asMilliseconds();
		clock.restart();

		info.window.setView(info.view);
		draw_map(&info, time, player_index);
		gui::instance().draw(&info);

	//	show_cursor_point(&info);

		if(!info.path.empty())
			draw_path(&info, info.path, 0);	//TO DO DELETE

		info.window.display();
	}

	return 0;
}
