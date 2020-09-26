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

	uint32_t player_index = add_player(&info, "Enemy");
	uint32_t player_index2 = add_player(&info, "Player");
	info.announce_war(player_index, player_index2);

	std::vector<std::shared_ptr<client> > clients{};
	clients.emplace_back(std::make_shared<client>(&info, 1400, 900, player_index));
	clients.emplace_back(std::make_shared<client>(&info, 1400, 900, player_index2));

	for(uint32_t i = 0; i < 19; ++i){
		uint32_t spawn_cell_index = choose_spawn_cell(&info);
		std::shared_ptr<unit> mech =
			mech::create(player_index, spawn_cell_index, &info.item_db);
		info.players[player_index].units.emplace_back(mech);
	}
	for(auto &unit : info.players[player_index].units)
		unit->open_vision(&info, player_index);

	std::vector<std::shared_ptr<ai> > aies{};
	aies.emplace_back(std::make_shared<ai>(&info, player_index));

	while(true){
		time = clock.getElapsedTime().asMilliseconds();
		clock.restart();

		for(auto& client : clients){
			client->control_update(&info, time);
			client->draw(&info, time);
		}

		if(info.pause == true)
			continue;

		float time_mod[3] = {1, 2, 4};
		time *= time_mod[info.speed];

		for(auto& ai : aies){
			ai->update(time);
		}

		info.update(time);
	}

	return 0;
}
