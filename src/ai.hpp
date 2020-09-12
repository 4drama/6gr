#include "map.hpp"

#include <functional>

class ai{
public:
	ai(game_info *info_, uint32_t player_index_);
	update();
private:
	game_info *info;
	uint32_t player_index;

	const player& player;

	enum class STATES{
		INIT,
		SCOUR,
		IDLE,

		SIZE
	};

	std::vector<std::pair<std::shared_ptr<unit>, STATES>> unit_pairs;

	static std::array< std::function<void(ai*, std::shared_ptr<unit>, STATES*)>,
		(std::size_t)STATES::SIZE> state_functions;
	static void state_functions_init();
};
