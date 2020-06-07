#include "control.hpp"

#include "gui.hpp"

#include <memory>

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

void set_speed(game_info *info, game_info::speed_e speed){
	info->pause = false;
	info->speed = speed;
}

void select_cell(game_info *info, uint32_t player_index, client* client){
	info->players[player_index].selected_cell = get_cell_index_under_mouse(info, client);
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

bool select_item(game_info *info, uint32_t player_index, client* client){
	if(info->players[player_index].selected_units.size() == 1){
		auto unit_ptr = info->players[player_index].selected_units.front().second.lock();
		return unit_ptr->interact_gui(info, client);
	}
	return false;
}
/*
void event_handler(game_info *info, float time, client *client){
	client->control_update(info, time);
}*/
