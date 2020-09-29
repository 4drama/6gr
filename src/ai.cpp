#include "ai.hpp"

#include "map.hpp"

#include <cstdlib>
#include <iostream>

std::list<std::shared_ptr<unit>> ai::get_viewed_enemy(
	std::shared_ptr<unit> unit_ptr, uint32_t depth = UINT32_MAX){

	std::list<std::shared_ptr<unit>> res{};

	area area_around = area(this->info, unit_ptr->cell_index, depth);
	std::vector<bool>* vision_map =
		get_vision_map(this->info, this->player.info->get_vision_players_indeces());
	std::list<uint32_t> seen_area = area_around.filter(vision_map);

	for(auto &cell : seen_area){
		if((this->info->map[cell].unit != nullptr)
			&& (this->info->map[cell].unit->player_index != UINT32_MAX)){

			uint32_t player_index = this->info->map[cell].unit->player_index;
			if(this->player.info->relationship[player_index]
				== player_info::relationship_type::ENEMY){

				res.emplace_back(this->info->map[cell].unit);
			}
		}
	}
	return res;
}

ai_state::ai_state(ai* ai_ptr, std::shared_ptr<unit> unit)
	: state_ptr(std::make_shared<ai_state_init>(ai_ptr, unit)){
}

void ai_state::update(float time){
	std::shared_ptr<ai_state_base> tmp_state = this->state_ptr->update_and_get(time);
	if(tmp_state != nullptr)
		this->state_ptr = tmp_state;
}

ai_state_init::ai_state_init(ai* ai_ptr, std::shared_ptr<unit> unit_ptr)
	: ai_state_base(ai_ptr, unit_ptr), delay(rand() % (600 * 1000)){
}

ai_state_scout::ai_state_scout(ai* ai_ptr, std::shared_ptr<unit> unit_ptr)
	: ai_state_base(ai_ptr, unit_ptr){
}

ai_state_idle::ai_state_idle(ai* ai_ptr, std::shared_ptr<unit> unit_ptr)
	: ai_state_base(ai_ptr, unit_ptr){
}

ai_go_to_target::ai_go_to_target(ai* ai_ptr, std::shared_ptr<unit> unit_ptr,
	std::shared_ptr<unit> target_ptr_)
	: ai_state_base(ai_ptr, unit_ptr),
	target_ptr(target_ptr_),
	last_target_position(target_ptr_->cell_index){
}

ai_attack::ai_attack(ai* ai_ptr, std::shared_ptr<unit> unit_ptr,
	std::shared_ptr<unit> target_ptr_)
	: ai_state_base(ai_ptr, unit_ptr),
	target_ptr(target_ptr_),
	last_target_position(target_ptr_->cell_index){
}

ai_run_away::ai_run_away(ai* ai_ptr, std::shared_ptr<unit> unit_ptr,
	std::shared_ptr<unit> target_ptr_)
	: ai_state_base(ai_ptr, unit_ptr),
	target_ptr(target_ptr_){

	uint32_t cell_index = rand() % ai_ptr->info->map.size();
	unit_ptr->update_path(ai_ptr->info, ai_ptr->player_index, cell_index);
}

ai_strafe::ai_strafe(ai* ai_ptr, std::shared_ptr<unit> unit_ptr,
	std::shared_ptr<unit> target_ptr_)
	: ai_state_base(ai_ptr, unit_ptr),
	target_ptr(target_ptr_),
	last_target_position(target_ptr_->cell_index){

	this->strafe_cell = ai_ptr->info->map[unit_ptr->cell_index].
		indeces[rand() % (int)cardinal_directions_t::END];

	unit_ptr->update_path(ai_ptr->info, ai_ptr->player_index, this->strafe_cell);
}

ai_state_find_in::ai_state_find_in(ai* ai_ptr, std::shared_ptr<unit> unit_ptr,
	uint32_t last_target_position_)
	: ai_state_base(ai_ptr, unit_ptr),
	last_target_position(last_target_position_){
	unit_ptr->update_path(ai_ptr->info, ai_ptr->player_index, last_target_position);
}

std::shared_ptr<ai_state_base> ai_state_init::update_and_get(float time){
	if((this->spend_time > this->delay) || (!ai_ptr->get_viewed_enemy(unit_ptr, 8).empty())){
		dynamic_cast<mech*>(this->unit_ptr.get())->system_on();
		return std::make_shared<ai_state_scout>(ai_ptr, unit_ptr);
	} else {
		this->spend_time += time;
		return nullptr;
	}
}

namespace{

bool condition_ready(mech* mech_ptr, float energy_rate, float heat_rate){
	return (mech_ptr->get_energy_rate() > energy_rate)
		&& (mech_ptr->get_heat_rate() < heat_rate);
}

bool condition_ready(std::shared_ptr<unit> unit_ptr, float energy_rate, float heat_rate){
	mech* mech_ptr = dynamic_cast<mech*>(unit_ptr.get());
	return condition_ready(mech_ptr, energy_rate, heat_rate);
}

}

std::shared_ptr<ai_state_base> ai_state_scout::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;
	for(auto &sel_unit : this->ai_ptr->player.selected_units){
		if(sel_unit.second.lock() == unit_ptr){
			std::cerr << "SCOUT" << std::endl;
		}
	}

	if(unit_ptr->path.empty()){
		uint32_t cell_index = rand() % ai_ptr->info->map.size();
		unit_ptr->update_path(ai_ptr->info, ai_ptr->player_index, cell_index);
	}

	if(!condition_ready(unit_ptr, 0.3, 0.6)){
		state_ptr = std::make_shared<ai_state_idle>(ai_ptr, unit_ptr);
	}

	std::list<std::shared_ptr<unit>> enemy_list = ai_ptr->get_viewed_enemy(unit_ptr, 6);
	if(!enemy_list.empty()){
		unit_ptr->path.clear();

		for(auto &enemy_unit : enemy_list){
			std::list<uint32_t> path = get_path(ai_ptr->info,
				unit_ptr->cell_index, enemy_unit->cell_index, UINT32_MAX);

			if(path.size() < 6){
				state_ptr = std::make_shared<ai_attack>(ai_ptr, unit_ptr, enemy_unit);
			} else {
				state_ptr = std::make_shared<ai_go_to_target>(ai_ptr, unit_ptr, enemy_unit);
			}

			std::cerr << "Enemy on :" << enemy_unit->cell_index
				<< ". In range: " << path.size() << std::endl;
		}
	}

	return state_ptr;
}

std::shared_ptr<ai_state_base> ai_state_idle::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;
	for(auto &sel_unit : this->ai_ptr->player.selected_units){
		if(sel_unit.second.lock() == unit_ptr){
			std::cerr << "IDLE" << std::endl;
		}
	}

	mech* mech_ptr = dynamic_cast<mech*>(unit_ptr.get());
	if(!mech_ptr->path.empty())
		mech_ptr->path.clear();

	if(condition_ready(unit_ptr, 0.65, 0.3)){
		state_ptr = std::make_shared<ai_state_scout>(ai_ptr, unit_ptr);
	}

	std::list<std::shared_ptr<unit>> enemy_list = ai_ptr->get_viewed_enemy(unit_ptr, 6);
	if(!enemy_list.empty()){
		if(condition_ready(unit_ptr, 0.5, 0.5)){
			state_ptr = std::make_shared<ai_go_to_target>(ai_ptr, unit_ptr, enemy_list.front());
		} else {
			state_ptr = std::make_shared<ai_run_away>(ai_ptr, unit_ptr, enemy_list.front());
		}
	}

	return state_ptr;
}

std::shared_ptr<ai_state_base> ai_go_to_target::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;
	for(auto &sel_unit : this->ai_ptr->player.selected_units){
		if(sel_unit.second.lock() == unit_ptr){
			std::cerr << "GO_TO_TARGET" << std::endl;
		}
	}

	std::vector<bool>* vision_map =
		get_vision_map(this->ai_ptr->info,
			this->ai_ptr->player.info->get_vision_players_indeces());

	if((*vision_map)[target_ptr->cell_index] && (target_ptr->player_index != UINT32_MAX)){
		this->last_target_position = target_ptr->cell_index;
		if(!condition_ready(unit_ptr, 0.3, 0.6)){
			state_ptr = std::make_shared<ai_run_away>(ai_ptr, unit_ptr, target_ptr);
		}

		std::list<uint32_t> path = get_path(ai_ptr->info,
			unit_ptr->cell_index, target_ptr->cell_index, UINT32_MAX);
		if(path.size() < ai_attack::attack_range){
			state_ptr = std::make_shared<ai_attack>(ai_ptr, unit_ptr, target_ptr);
		} else {
			unit_ptr->update_path(
				ai_ptr->info, ai_ptr->player_index, target_ptr->cell_index);
		}
	} else {
		state_ptr = std::make_shared<ai_state_find_in>(
			ai_ptr, unit_ptr, this->last_target_position);
	}

	return state_ptr;
}

std::shared_ptr<ai_state_base> ai_attack::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;
	for(auto &sel_unit : this->ai_ptr->player.selected_units){
		if(sel_unit.second.lock() == unit_ptr){
			std::cerr << "ATTACK" << std::endl;
		}
	}

	std::vector<bool>* vision_map =
		get_vision_map(this->ai_ptr->info,
			this->ai_ptr->player.info->get_vision_players_indeces());

	if(!condition_ready(unit_ptr, 0.3, 0.6)){
		if((*vision_map)[target_ptr->cell_index] && (target_ptr->player_index != UINT32_MAX)){
			state_ptr = std::make_shared<ai_run_away>(ai_ptr, unit_ptr, target_ptr);
		} else {
			state_ptr = std::make_shared<ai_state_idle>(ai_ptr, unit_ptr);
		}
	}

	if((*vision_map)[target_ptr->cell_index] && (target_ptr->player_index != UINT32_MAX)){
		this->last_target_position = target_ptr->cell_index;
		std::list<uint32_t> path = get_path(ai_ptr->info,
			unit_ptr->cell_index, target_ptr->cell_index, UINT32_MAX);

		if(path.size() < ai_attack::attack_range){
			mech* mech_ptr = dynamic_cast<mech*>(unit_ptr.get());

			for(auto &weapon_ptr : mech_ptr->get_ready_weapons()){
				weapon_ptr->use(ai_ptr->info, mech_ptr, target_ptr->cell_index);
				break;
			}
			state_ptr = std::make_shared<ai_strafe>(ai_ptr, unit_ptr, target_ptr);
		} else {
			state_ptr = std::make_shared<ai_go_to_target>(ai_ptr, unit_ptr, target_ptr);
		}
	} else {
		state_ptr = std::make_shared<ai_state_find_in>(
			ai_ptr, unit_ptr, this->last_target_position);
	}

	return state_ptr;
}

std::shared_ptr<ai_state_base> ai_run_away::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;
	for(auto &sel_unit : this->ai_ptr->player.selected_units){
		if(sel_unit.second.lock() == unit_ptr){
			std::cerr << "RUN_AWAY: time - " << this->after_warning_time << std::endl;
		}
	}

	if(unit_ptr->path.empty()){
		uint32_t cell_index = rand() % ai_ptr->info->map.size();
		unit_ptr->update_path(ai_ptr->info, ai_ptr->player_index, cell_index);
	}

	std::list<std::shared_ptr<unit>> enemy_list = ai_ptr->get_viewed_enemy(unit_ptr, 6);
	if(enemy_list.empty()){
		this->after_warning_time += time;
		if(this->after_warning_time >= 16 * 1000){
			state_ptr = std::make_shared<ai_state_idle>(ai_ptr, unit_ptr);
		}
	}

	return state_ptr;
}

std::shared_ptr<ai_state_base> ai_strafe::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;
	for(auto &sel_unit : this->ai_ptr->player.selected_units){
		if(sel_unit.second.lock() == unit_ptr){
			std::cerr << "STRAFE" << std::endl;
		}
	}

	std::vector<bool>* vision_map =
		get_vision_map(this->ai_ptr->info,
			this->ai_ptr->player.info->get_vision_players_indeces());

	if((*vision_map)[target_ptr->cell_index]){
		this->last_target_position = target_ptr->cell_index;
		if(unit_ptr->path.empty()){
			state_ptr = std::make_shared<ai_go_to_target>(ai_ptr, unit_ptr, target_ptr);
		}
	} else {
		state_ptr = std::make_shared<ai_state_find_in>(
			ai_ptr, unit_ptr, this->last_target_position);
	}

	return state_ptr;
}

std::shared_ptr<ai_state_base> ai_state_find_in::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;
	for(auto &sel_unit : this->ai_ptr->player.selected_units){
		if(sel_unit.second.lock() == unit_ptr){
			std::cerr << "FIND_IN" << std::endl;
		}
	}

	std::list<std::shared_ptr<unit>> enemy_list = ai_ptr->get_viewed_enemy(unit_ptr, 6);
	if(!enemy_list.empty()){
		unit_ptr->path.clear();

		for(auto &enemy_unit : enemy_list){
			std::list<uint32_t> path = get_path(ai_ptr->info,
				unit_ptr->cell_index, enemy_unit->cell_index, UINT32_MAX);

			if(path.size() < 6){
				state_ptr = std::make_shared<ai_attack>(ai_ptr, unit_ptr, enemy_unit);
			} else {
				state_ptr = std::make_shared<ai_go_to_target>(ai_ptr, unit_ptr, enemy_unit);
			}
		}
	}

	if(unit_ptr->path.empty()){
		state_ptr = std::make_shared<ai_state_scout>(ai_ptr, unit_ptr);
	}

	return state_ptr;
}

ai::ai(game_info *info_, uint32_t player_index_)
	: info(info_), player_index(player_index_),
	player(info_->players[player_index_]){

	for(auto &unit : this->player.units){
		unit_states.emplace_back(this, unit);
	}
};

ai::update(float time){
	for(auto it = unit_states.begin(); it != unit_states.end(); ++it){
		if(it->is_valid())
			it->update(time);
		else
			unit_states.erase(it--);
	}
};
