#include "map.hpp"

#include <functional>

class ai;
class ai_state_base;

class ai_state{
public:
	ai_state() = delete;
	ai_state(ai* ai_ptr, std::shared_ptr<unit> unit);
	void update(float time);
private:
	std::shared_ptr<ai_state_base> state_ptr;
};

class ai_state_base : public std::enable_shared_from_this<ai_state_base>{
public:
	ai_state_base() = delete;
	ai_state_base(ai* ai_ptr_, std::shared_ptr<unit> unit_) : unit_ptr(unit_),
		ai_ptr(ai_ptr_){};

	[[nodiscard]] virtual std::shared_ptr<ai_state_base> update_and_get(float time) = 0;
	const std::shared_ptr<unit>& get_unit_ptr() const noexcept{return this->unit_ptr;};
protected:
	std::shared_ptr<unit> unit_ptr;
	ai* ai_ptr;
};

class ai_state_init : public ai_state_base{
public:
	ai_state_init(ai* ai_ptr, std::shared_ptr<unit> unit_ptr);
	std::shared_ptr<ai_state_base> update_and_get(float time) override;
};

class ai_state_scout : public ai_state_base{
public:
	ai_state_scout(ai* ai_ptr, std::shared_ptr<unit> unit_ptr);
	std::shared_ptr<ai_state_base> update_and_get(float time) override;
};

class ai_state_idle : public ai_state_base{
public:
	ai_state_idle(ai* ai_ptr, std::shared_ptr<unit> unit_ptr);
	std::shared_ptr<ai_state_base> update_and_get(float time) override;
};

class ai{
public:
	ai(game_info *info_, uint32_t player_index_);
	update(float time);
private:
	game_info *info;
	uint32_t player_index;

	const player& player;

	std::vector<ai_state> unit_states;

	friend class ai_state_init;
	friend class ai_state_scout;
	friend class ai_state_idle;
};
