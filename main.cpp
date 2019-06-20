#include <SFML/Graphics.hpp>

#include <cmath>
#include <vector>
#include <cstdlib>

#include "map.hpp"
#include "control.hpp"

#include <iostream>

int main(){
	game_info info{};
	sf::Clock clock;
	float time;

	while(info.window.isOpen()){
		event_handler(&info);

		info.window.clear();
		time = clock.getElapsedTime().asMilliseconds();
		clock.restart();

		draw_map(&info, time);

		info.window.setView(info.view);
		info.window.display();
	}

	return 0;
}
