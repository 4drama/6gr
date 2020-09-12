#include "ai.hpp"

std::array< std::function<void(ai*, std::shared_ptr<unit>, ai::STATES*)>,
	(std::size_t)ai::STATES::SIZE> ai::state_functions{};

void ai::state_functions_init(){
	static bool is_done = false;
	if(!is_done){
		ai::state_functions[(std::size_t)STATES::INIT] = []
			(ai* ai_ptr, std::shared_ptr<unit> unit_ptr, STATES* state_ptr){

			dynamic_cast<mech*>(unit_ptr.get())->system_on();
			*state_ptr = STATES::SCOUR;
		};

		ai::state_functions[(std::size_t)STATES::SCOUR] = []
			(ai* ai_ptr, std::shared_ptr<unit> unit_ptr, STATES* state_ptr){
			if(unit_ptr->path.empty()){
				uint32_t cell_index = rand() % ai_ptr->info->map.size();
				unit_ptr->update_path(ai_ptr->info, ai_ptr->player_index, cell_index);
			}

			mech* mech_ptr = dynamic_cast<mech*>(unit_ptr.get());
			if((mech_ptr->get_energy_rate() < 0.3) ||
				(mech_ptr->get_heat_rate() > 0.7)){
				*state_ptr = STATES::IDLE;
			}
		};

		ai::state_functions[(std::size_t)STATES::IDLE] = []
			(ai* ai_ptr, std::shared_ptr<unit> unit_ptr, STATES* state_ptr){
			mech* mech_ptr = dynamic_cast<mech*>(unit_ptr.get());
			if(!mech_ptr->path.empty())
				mech_ptr->path.clear();

			if((mech_ptr->get_energy_rate() > 0.65) &&
				(mech_ptr->get_heat_rate() < 0.3)){
				*state_ptr = STATES::SCOUR;
			}
		};
		is_done = true;
	}
}

ai::ai(game_info *info_, uint32_t player_index_)
	: info(info_), player_index(player_index_),
	player(info_->players[player_index_]){
	ai::state_functions_init();

	for(auto &unit : this->player.units){
		this->unit_pairs.emplace_back(unit, STATES::INIT);
	}
};

ai::update(){
	for(auto &unit_pair : unit_pairs){
		std::shared_ptr<unit> &unit = unit_pair.first;
		ai::STATES* state_ptr = &unit_pair.second;
		state_functions[(std::size_t)*state_ptr](this, unit, state_ptr);
	}
};
