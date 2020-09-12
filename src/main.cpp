#include <SFML/Graphics.hpp>

#include <cmath>
#include <vector>
#include <cstdlib>

#include "map.hpp"
#include "control.hpp"
#include "gui.hpp"
#include "ai.hpp"

#include <iostream>
#include <memory>

int main(){
	sf::Clock clock;
	float time;
	game_info info{};

	uint32_t player_index = add_player(&info, "Player 1");
	uint32_t player_index2 = add_player(&info, "Player 2");
	info.announce_war(player_index, player_index2);

	std::vector<std::shared_ptr<client> > clients{};
	clients.emplace_back(std::make_shared<client>(&info, 1400, 900, player_index));
	clients.emplace_back(std::make_shared<client>(&info, 1400, 900, player_index2));

	std::vector<std::shared_ptr<ai> > aies{};
	aies.emplace_back(std::make_shared<ai>(&info, player_index));

	while(true){
		time = clock.getElapsedTime().asMilliseconds();
		clock.restart();

		for(auto& client : clients){
			client->control_update(&info, time);
			client->draw(&info, time);
		}

		for(auto& ai : aies){
			ai->update();
		}

		info.update(time);
	}

	return 0;
}
