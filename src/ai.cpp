#include "ai.hpp"

ai_state::ai_state(ai* ai_ptr, std::shared_ptr<unit> unit)
	: state_ptr(std::make_shared<ai_state_init>(ai_ptr, unit)){
}

void ai_state::update(float time){
	std::shared_ptr<ai_state_base> tmp_state = this->state_ptr->update_and_get(time);
	if(tmp_state != nullptr)
		this->state_ptr = tmp_state;
}

ai_state_init::ai_state_init(ai* ai_ptr, std::shared_ptr<unit> unit_ptr)
	: ai_state_base(ai_ptr, unit_ptr){
}

ai_state_scout::ai_state_scout(ai* ai_ptr, std::shared_ptr<unit> unit_ptr)
	: ai_state_base(ai_ptr, unit_ptr){
}

ai_state_idle::ai_state_idle(ai* ai_ptr, std::shared_ptr<unit> unit_ptr)
	: ai_state_base(ai_ptr, unit_ptr){
}

std::shared_ptr<ai_state_base> ai_state_init::update_and_get(float time){
	dynamic_cast<mech*>(this->unit_ptr.get())->system_on();
	return std::make_shared<ai_state_scout>(ai_ptr, unit_ptr);
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
	for(auto &state : unit_states){
		state.update(time);
	}
};
