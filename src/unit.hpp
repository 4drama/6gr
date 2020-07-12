#ifndef UNIT_HPP
#define UNIT_HPP

#include <SFML/Graphics.hpp>

#include "items.hpp"
#include "map.hpp"

#include <map>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <functional>

class item;
class legs;
class engine;
class weapon;
class torpedo_info;

class projectile{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	projectile(game_info *info, std::list<uint32_t> path, uint32_t cell_index,
		uint32_t aoe, std::shared_ptr<torpedo_info> torpedo_info_ptr);

	inline bool is_explosion() const noexcept {return this->explosion;};
	inline uint32_t get_aoe() const noexcept {return this->aoe;};
	inline uint32_t get_cell_index() const noexcept {return this->cell_index;};
	void update(game_info *info, float time);

	void draw(game_info *info, client *client) const noexcept;
/*	const std::shared_ptr<torpedo_info>& get_info() const noexcept{
		return this->torpedo_info_ptr;};*/
	void detonate(game_info *info) const;
private:
	static constexpr float scale = 0.2;
	std::shared_ptr<torpedo_info> torpedo_info_ptr;

	uint32_t aoe;
	float speed = 10;
	bool explosion = false;
//	uint32_t lifetime = 4;

	uint32_t start_cell_index;
	uint32_t cell_index;
	uint32_t target_cell_index() const noexcept {return path.back();};

	float progress = 0.0f;
	float length;
	float angle = 0.0f;

	std::list<uint32_t> path;
	float path_progress = 0;
};

struct unit : std::enable_shared_from_this<unit>{
	static std::map<std::string, sf::Texture> textures;

	uint32_t cell_index;
	std::vector<sf::Sprite> sprites;

	uint32_t vision_range = 0;
	std::vector<uint32_t> vision_indeces{};

	std::list<uint32_t> path;
	float path_progress = 0;

	unit(uint32_t cell_index, uint32_t vision_range);

	void open_vision(game_info *info, uint32_t player_index);
	void update(game_info *info, uint32_t player_index, float time);

	inline virtual float get_speed(terrain_en ter_type) const noexcept {return 0;};
	virtual void draw_gui(game_info *info, client *client){return ;};
	virtual bool interact_gui(game_info *info, client *client){return false;};

	virtual bool event(sf::Event event, game_info *info,
		uint32_t player_index, uint32_t target_cell) noexcept;

	virtual bool damage(float damage) noexcept {return false;};

private:
	void unit_update_move(game_info *info, uint32_t player_index, float time);
	inline virtual void update_v(game_info *info, uint32_t player_index, float time){return ;};
	virtual float move_calculate(float time, terrain_en ter_type) noexcept{
		return this->get_speed(ter_type) * time;};

protected:
	void update_path(game_info *info, uint32_t player_index, uint32_t target_cell);
};

struct mech_status {
	enum class type{
		energy = 0,
		heat = 1,
		fuel = 2,
		size = 3
	};

	float current_energy = 0;
	float energy_capacity = 0;

	float current_heat = 0;
	float heat_capacity = 0;

	float current_fuel = 0;
	float fuel_capacity = 0;

	inline std::list<std::pair<mech_status::type, float>> get() const;

	bool is_useful(mech_status::type type, float value) const noexcept;
	bool is_full(mech_status::type type) const noexcept;
/*	bool is_empty(mech_status::type type) const noexcept;*/

	inline static mech_status zero(){return mech_status{0, 0, 0, 0, 0, 0};};

	inline static mech_status capacity(float energy, float heat, float fuel = 0){
		return mech_status{0, energy, 0, heat, 0, fuel};};

	inline static mech_status current(float energy, float heat, float fuel = 0){
		return mech_status{energy, 0, heat, 0, fuel, 0};};

	mech_status try_spend(const mech_status& right);

	std::pair<mech_status::type, float>
	try_spend(const std::pair<mech_status::type, float> &val);

	inline mech_status& operator*(float right);
	inline mech_status& add_capacity(const mech_status& right);
	inline mech_status& add_current(const mech_status& right);
	mech_status& add_current(const std::pair<mech_status::type, float> &val);

	friend inline const mech_status operator+(const mech_status& left,
		const mech_status& right);
	friend inline mech_status& operator+=(mech_status& left, const mech_status& right);

	friend inline const mech_status operator/(const mech_status& left, uint32_t value);
	friend inline mech_status& operator/=(mech_status& left, uint32_t value);

	inline operator bool() const;
	inline void clear_capacity() noexcept;
};

bool is_store(const std::pair<mech_status::type, float> &val);
inline bool is_spend(const std::pair<mech_status::type, float> &val){ return !is_store(val);}

struct part_of_mech{
	part_of_mech(float durability, float weight, uint32_t slots, float priority);
	void prepare_for_refresh() noexcept;
	bool add_item(std::shared_ptr<item> item);
	void validate();

	float durability = 30;
	float max_durability = 30;
	mutable sf::Text durability_text;

	mech_status status;
	std::list<std::shared_ptr<item>> items;

	float priority = 1.0f;

	float weight = 0;
	uint32_t slots = 0;

	struct{
		float weight;
		uint32_t slots;
	} limits;
};

class mech : public unit{
public:
	static inline std::shared_ptr<mech> create(uint32_t cell_index){
		return std::make_shared<mech>(cell_index);};

	mech(uint32_t cell_index);

	float get_speed(terrain_en ter_type) const noexcept override;

	void draw_gui(game_info *info, client *client) override;
	bool interact_gui(game_info *info, client *client) override;
	bool event(sf::Event event, game_info *info,
		uint32_t player_index, uint32_t target_cell) noexcept override;

	bool damage(float damage) noexcept override;

	float get_available_rate(mech_status necessary) const noexcept;

	inline void set_waiting_item(item *item) noexcept{this->waiting_confirm = item;};
	inline item *get_waiting_item() const noexcept{return waiting_confirm;};

	bool try_spend(const mech_status &status) noexcept;
	bool try_loading_torpedo(weapon* weapon_ptr);
private:
	item *waiting_confirm = nullptr;

	mutable sf::Text energy_text;
	mutable sf::Text heat_text;
	mutable sf::Text fuel_text;

	legs *legs_ptr = nullptr;
	engine *engine_ptr = nullptr;

	part_of_mech left_arm;
	part_of_mech torso;
	part_of_mech right_arm;
	mech_status accumulate_status() const noexcept;
	void calculate_status(const mech_status &status);

	item_shape get_status_shape(client *client, const sf::Vector2f& position) const;
	void update_v(game_info *info, uint32_t player_index, float time);

	item_shape prepare_shape(client *client) const;
	void refresh();

	float move_calculate(float time, terrain_en ter_type) noexcept override;
};

inline std::list<std::pair<mech_status::type, float>> mech_status::get() const{
	std::list<std::pair<mech_status::type, float>> result{};
	if(this->current_energy != 0)
		result.emplace_back(mech_status::type::energy, this->current_energy);
	if(this->current_heat != 0)
		result.emplace_back(mech_status::type::heat, this->current_heat);
	if(this->current_fuel != 0)
		result.emplace_back(mech_status::type::fuel, this->current_fuel);
	return result;
}

inline mech_status& mech_status::operator*(float right){
	this->current_energy *= right;
	this->current_heat *= right;
	this->current_fuel *= right;
	return *this;
}

inline mech_status& mech_status::add_capacity(const mech_status& right){
	this->energy_capacity += right.energy_capacity;
	this->heat_capacity += right.heat_capacity;
	this->fuel_capacity += right.fuel_capacity;
	return *this;
};

inline mech_status& mech_status::add_current(const mech_status& right){
	this->current_energy += right.current_energy;
	this->current_heat += right.current_heat;
	this->current_fuel += right.current_fuel;
	return *this;
};

inline const mech_status operator+(const mech_status& left, const mech_status& right){
	mech_status status(left);
	status.add_capacity(right);
	status.add_current(right);
	return status;
}

inline mech_status& operator+=(mech_status& left, const mech_status& right){
	return left = left + right;
}

inline const mech_status operator/(const mech_status& left, uint32_t value){
	mech_status status(left);
	status.current_energy /= value;
	status.current_heat /= value;
	status.current_fuel /= value;
	return status;
}

inline mech_status& operator/=(mech_status& left, uint32_t value){
	return left = left / value;
}

inline bool operator==(const mech_status& left, const mech_status& right){
	return (left.current_energy == right.current_energy) &&
		(left.current_heat == right.current_heat) &&
		(left.current_fuel == right.current_fuel) &&
		(left.energy_capacity == right.energy_capacity) &&
		(left.heat_capacity == right.heat_capacity) &&
		(left.fuel_capacity == right.fuel_capacity);
}

inline bool operator!=(const mech_status& left, const mech_status& right){
	return !(left == right);
}

inline mech_status::operator bool() const{
	return *this != mech_status::zero();
}

inline void mech_status::clear_capacity() noexcept{
	this->energy_capacity = 0;
	this->heat_capacity = 0;
	this->fuel_capacity = 0;
}

#endif
