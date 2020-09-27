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
		if(this->info->map[cell].unit != nullptr){
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

std::shared_ptr<ai_state_base> ai_state_init::update_and_get(float time){
	if((this->spend_time > this->delay) || (!ai_ptr->get_viewed_enemy(unit_ptr, 8).empty())){
		dynamic_cast<mech*>(this->unit_ptr.get())->system_on();
		return std::make_shared<ai_state_scout>(ai_ptr, unit_ptr);
	} else {
		this->spend_time += time;
		return nullptr;
	}
}

std::shared_ptr<ai_state_base> ai_state_scout::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;

	if(unit_ptr->path.empty()){
		uint32_t cell_index = rand() % ai_ptr->info->map.size();
		unit_ptr->update_path(ai_ptr->info, ai_ptr->player_index, cell_index);
	}

	mech* mech_ptr = dynamic_cast<mech*>(unit_ptr.get());
	if((mech_ptr->get_energy_rate() < 0.3) ||
		(mech_ptr->get_heat_rate() > 0.7)){
		state_ptr = std::make_shared<ai_state_idle>(ai_ptr, unit_ptr);
	}

	std::list<std::shared_ptr<unit>> enemy_list = ai_ptr->get_viewed_enemy(unit_ptr, 6);
	if(!enemy_list.empty()){
		for(auto &enemy_unit : enemy_list){
			std::list<uint32_t> path = get_path(ai_ptr->info,
				unit_ptr->cell_index, enemy_unit->cell_index, UINT32_MAX);

		/*	TO DO

			if(in range){
				atack
			} else
				go to target */

			std::cerr << "Enemy on :" << enemy_unit->cell_index
				<< ". In range: " << path.size() << std::endl;
		}
	}

	return state_ptr;
}

std::shared_ptr<ai_state_base> ai_state_idle::update_and_get(float time){
	std::shared_ptr<ai_state_base> state_ptr = nullptr;

	mech* mech_ptr = dynamic_cast<mech*>(unit_ptr.get());
	if(!mech_ptr->path.empty())
		mech_ptr->path.clear();

	if((mech_ptr->get_energy_rate() > 0.65) &&
		(mech_ptr->get_heat_rate() < 0.3)){
		state_ptr = std::make_shared<ai_state_scout>(ai_ptr, unit_ptr);
	}

	/* TO DO :

		if(seen(enemy))
			state = run away;
	*/

	return state_ptr;
}

/*	TO DO :
	state find_in_zone;
	update{

	}

	state go_to_target;
	update{
		if(!seen(target))
			state = scout / find in zone;

		if(seen(in_range(target)))
			state = attack;

		go to target
	}

	state attack;
	update{
		if(weapon_ready)
			shoot

		if(seen(!in_range(target)))
			state = go_to_target;

		if(!seen(target))
			state = scout / find in zone;

		if(warning)
			state = run away

		else
			state = strafe
	}

	state strafe;
	update{
		if(weapon_ready)
			state = attack

		strafe
	}

	state run_away;
	update{
		if(!warning)
			state = idle

		run
	}

*/

ai::ai(game_info *info_, uint32_t player_index_)
	: info(info_), player_index(player_index_),
	player(info_->players[player_index_]){

	for(auto &unit : this->player.units){
		unit_states.emplace_back(this, unit);
	}
};

ai::update(float time){
	for(auto &state : unit_states){
		state.update(time);
	}
};
