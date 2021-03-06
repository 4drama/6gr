#include "items.hpp"

#include "math.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <cassert>

sf::Texture weapon::texture{};
std::map<std::string, sf::Sprite> weapon::sprites{};

sf::Texture legs::texture{};
std::map<std::string, sf::Sprite> legs::sprites{};

sf::Texture engine::texture{};
std::map<std::string, sf::Sprite> engine::sprites{};

sf::Texture turn_on::texture{};
std::map<std::string, sf::Sprite> turn_on::sprites{};

sf::Texture cooling_system::texture{};
std::map<std::string, sf::Sprite> cooling_system::sprites{};

namespace{

void set_scale_f(float scale, std::map<std::string, sf::Sprite> &sprites){
	for(auto &sprite : sprites){
		sprite.second.setScale(scale, -scale);
	}
};

sf::Text *update_text_f(float scale, sf::Vector2f pos, sf::Text *text,
	const sf::Color &color = sf::Color::White){

	text->setScale(scale, -scale);
	text->setPosition(pos);
	text->setColor(color);

	return text;
};

void place_shape_f(item_shape &shape, float scale, const sf::Vector2f& position){
	for(auto& button : shape.elements){
		button.sprite.setPosition(button.sprite.getPosition() * scale + position * scale);
	}
	for(auto& text : shape.text_elements){
		text->setPosition(text->getPosition() * scale + position * scale);
	}
};

}

turn_on::turn_on(bool power_status_ = false)
	: power_status(power_status_){

	if(turn_on::sprites.empty())
		turn_on::load_turn_on_sprite();
}

void turn_on::load_turn_on_sprite(){
	turn_on::texture.loadFromFile("./../data/item_turn_on.png");

	sprites["power_off"] = sf::Sprite(turn_on::texture,
		sf::IntRect(0, 0, 28, 35));
	sprites["power_off"].setPosition(-13, 0);

	sprites["power_on"] = sf::Sprite(turn_on::texture,
		sf::IntRect(28, 0, 28, 35));
	sprites["power_on"].setPosition(-13, 0);
};

item_shape turn_on::get_draw_shape(const mech* owner, client *client,
	const sf::Vector2f& position){

	float scale = client->get_view_scale();
	set_scale_f(scale, turn_on::sprites);

	item_shape shape{};
	if(this->get_power_status()){
		shape.elements.emplace_back(turn_on::sprites["power_on"],
			[this](){this->power_switch(false);});
	} else {
		shape.elements.emplace_back(turn_on::sprites["power_off"],
			[this](){this->power_switch(true);});
	}

	place_shape_f(shape, scale, position);
	return shape;
}

weapon::weapon(uint32_t id, deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, std::string name,
	float weight, uint32_t slots, float delay_)
	: item(id, part_ptr, name, weight, slots), delay(delay_ / 10000),
	text_ptr(create_text(text_delete_contaier, name, get_font(), 20)),
	shot_energy(-25), shot_heat(40), delay_energy(-5){
	if(weapon::sprites.empty())
		weapon::load_sprites();
	this->text_ptr->setPosition(39, 5);
}

bool weapon::has_resources(const mech* owner) const noexcept{
	const auto status = mech_status::current(this->shot_energy, this->shot_heat);
	if((owner->get_available_rate(status) == 1.0f) && torpedo_info_ptr){
		return true;
	} else
		return false;
};

void weapon::use(game_info *info, mech* owner, uint32_t target_cell){
	if(target_cell == UINT32_MAX)
		return ;

	const auto status = mech_status::current(this->shot_energy, this->shot_heat);
	if(this->get_ready(owner) && owner->try_spend(status)){
		std::list<uint32_t> path =
			get_path(info, owner->cell_index, target_cell, this->range);

		curr_delay = 0;
		this->torpedo_info_ptr->create_projectile(info, owner, path,
			target_cell, this->torpedo_info_ptr);
		this->torpedo_info_ptr = nullptr;
	}
}

bool weapon::get_ready(const mech* owner) const noexcept{
	bool status = false;
	if(this->get_power_status() && (this->get_delay() >= 1) &&
		this->has_resources(owner) && this->is_working()){
		status = true;
	}
	return status;
}

void weapon::update(mech* owner, float time){
	if(this->get_power_status() && this->is_working()){
		if(!torpedo_info_ptr){
			owner->try_loading_torpedo(this);
		}

		if(this->curr_delay < this->delay){

			if((this->curr_delay + time) > this->delay){
				float diff = this->delay - (this->curr_delay + time);
				time -= diff;
			}
			auto status =
			mech_status::current(this->delay_energy * time, 0);
			float rate = owner->get_available_rate(status);

			if((rate != 0) && owner->try_spend(status * rate))
			this->curr_delay += time * rate;
		}
	}
}

damage_info::damage_info(std::map<part_t, damage_to_part_t> damages_)
	: damages(damages_){
}

damage_info::damage_t damage_info::get(damage_info::part_t part) const noexcept{
	try{
		const damage_info::damage_to_part_t& damage_part = this->damages.at(part);
		const float accuracy = (float)(std::rand() % 100) / 100.f;

		return (std::rand() % 100) < damage_part.chance ?
		damage_info::damage_t{
			damage_part.damage.first + accuracy * damage_part.damage.second,
			damage_part.heat.first + accuracy * damage_part.heat.second} :
			damage_info::damage_t{0, 0};

	} catch(const std::exception& e){
		std::cerr << "damage_info::get(part_t part) part not found." << std::endl;
		return damage_info::damage_t{0, 0};
	}
}

namespace{

damage_info create_explosive_torpedo_damage_f(float min_damage, float max_damage,
	float min_heat, float max_heat){
	using part_t = damage_info::part_t;
	using damage_t = damage_info::damage_t;
	using damage_to_part_t = damage_info::damage_to_part_t;
	std::map<part_t, damage_to_part_t> damages{};
	damages[part_t::TORSO] = damage_to_part_t{
		100, {min_damage, max_damage - min_damage},
		{min_heat, max_heat - min_heat}};
	damages[part_t::LEFT_ARM] = damage_to_part_t{
		100, {min_damage, max_damage - min_damage},
		{min_heat, max_heat - min_heat}};
	damages[part_t::RIGHT_ARM] = damage_to_part_t{
		100, {min_damage, max_damage - min_damage},
		{min_heat, max_heat - min_heat}};
	return damage_info(damages);
}

}

explosive_torpedo::explosive_torpedo(){
	damage.emplace_back(create_explosive_torpedo_damage_f(30, 150, 10, 75));
	damage.emplace_back(create_explosive_torpedo_damage_f(15, 75, 5, 32.5f));
}

void explosive_torpedo::create_projectile(game_info *info, mech* owner,
	std::list<uint32_t> path, uint32_t target_cell,
	std::shared_ptr<torpedo_info> torpedo_info_ptr) const{

	info->add_projectile( std::make_shared<projectile>(info, path, owner->cell_index,
		this->aoe, torpedo_info_ptr));
}

area explosive_torpedo::get_damage_zone(game_info *info, uint32_t target_cell) const {
	return area(info, target_cell, this->aoe);
}

void explosive_torpedo::detonate(game_info *info, uint32_t target_cell) const {
	area area = this->get_damage_zone(info, target_cell);

	assert(this->aoe + 1 == this->damage.size());
	for(uint32_t i = 0; i < this->damage.size(); ++i){
		for(const auto& cell_index : area.get_level(i)){
			auto &unit = info->get_cell(cell_index).unit;
			if(unit){
				unit->damage(this->damage[i]);
			}
		}
	}
}

void weapon::load_sprites(){
	weapon::texture.loadFromFile("./../data/weapon_button.png");

	sprites["active_button"] = sf::Sprite(weapon::texture,
		sf::IntRect(0, 0, 180, 23));
	sprites["active_button"].setPosition(34, 0);

	sprites["inactive_button"] = sf::Sprite(weapon::texture,
		sf::IntRect(0, 23, 180, 23));
	sprites["inactive_button"].setPosition(34, 0);

	sprites["active_hotkey_screen"] = sf::Sprite(weapon::texture,
		sf::IntRect(180, 0, 15, 23));
	sprites["active_hotkey_screen"].setPosition(17, 0);

	sprites["inactive_hotkey_screen"] = sf::Sprite(weapon::texture,
		sf::IntRect(180, 23, 15, 23));
	sprites["inactive_hotkey_screen"].setPosition(17, 0);

	sprites["active_delay_screen"] = sf::Sprite(weapon::texture,
		sf::IntRect(0, 46, 197, 10));
	sprites["active_delay_screen"].setPosition(17, -25);

	sprites["inactive_delay_screen"] = sf::Sprite(weapon::texture,
		sf::IntRect(0, 56, 197, 10));
	sprites["inactive_delay_screen"].setPosition(17, -25);

	sprites["progress_bar_ready"] = sf::Sprite(weapon::texture,
		sf::IntRect(195, 0, 35, 6));
	sprites["progress_bar_ready"].setPosition(22, -27);

	sprites["progress_bar_not_ready"] = sf::Sprite(weapon::texture,
		sf::IntRect(195, 6, 35, 6));
	sprites["progress_bar_not_ready"].setPosition(22, -27);

	sprites["progress_bar_reload_ready"] = sf::Sprite(weapon::texture,
		sf::IntRect(195, 12, 35, 6));
	sprites["progress_bar_reload_ready"].setPosition(22, -27);
}

item::item(uint32_t id_, const part_of_mech *part_ptr_,
	std::string name_, float weight_, uint32_t slots_)
	: id(id_), name(name_), weight(weight_), slots(slots_), part_ptr(part_ptr_){
}

bool item::is_working() const noexcept{
	assert(this->part_ptr);
	return this->part_ptr->durability > 0 ? true : false;
}

item_shape weapon::get_draw_shape(const mech* owner, client *client,
	const sf::Vector2f& position){

	float scale = client->get_view_scale();
	set_scale_f(scale, weapon::sprites);

	item_shape shape{};
	if(this->get_power_status()){
		shape.elements.emplace_back(weapon::sprites["active_hotkey_screen"], std::function<void()>());
		shape.elements.emplace_back(weapon::sprites["active_delay_screen"], std::function<void()>());

		if(this->get_ready(owner)){
			for(uint32_t x = 0; x < 190; x += 38){
				item_button button(weapon::sprites["progress_bar_ready"], std::function<void()>());
				button.sprite.setPosition(button.sprite.getPosition() + sf::Vector2f(x, 0));
				shape.elements.emplace_back(button);
			}
			shape.text_elements.emplace_back(
				update_text_f(scale, sf::Vector2f(39, 5), this->text_ptr.get()));
			shape.elements.emplace_back(weapon::sprites["active_button"],
				[owner, this](){const_cast<mech*>(owner)->set_waiting_item(this);});
		} else {
			shape.elements.emplace_back(weapon::sprites["inactive_button"], std::function<void()>());
			float delay_rate = this->get_delay();
			float progress = 0;
			for(uint32_t x = 0; x < 190; x += 38){
				progress += 0.2;
				if(progress < delay_rate){
					item_button button(weapon::sprites["progress_bar_reload_ready"], std::function<void()>());
					button.sprite.setPosition(button.sprite.getPosition() + sf::Vector2f(x, 0));
					shape.elements.emplace_back(button);
				} else {
					item_button button(weapon::sprites["progress_bar_not_ready"], std::function<void()>());
					button.sprite.setPosition(button.sprite.getPosition() + sf::Vector2f(x, 0));
					shape.elements.emplace_back(button);
				}
			}
			shape.text_elements.emplace_back(update_text_f(
				scale, sf::Vector2f(40, 5), this->text_ptr.get(), sf::Color(140, 136, 136)));
		}
	} else {
		shape.elements.emplace_back(weapon::sprites["inactive_hotkey_screen"], std::function<void()>());
		shape.elements.emplace_back(weapon::sprites["inactive_button"], std::function<void()>());
		shape.elements.emplace_back(weapon::sprites["inactive_delay_screen"], std::function<void()>());
		shape.text_elements.emplace_back(update_text_f(
			scale,sf::Vector2f(40, 5), this->text_ptr.get(), sf::Color(140, 136, 136)));
	}

	place_shape_f(shape, scale, position);
	return shape;
}

void weapon::draw_active_zone(uint32_t mech_cell_position, game_info *info, client *client){
	uint32_t target = get_cell_index_under_mouse(info, client);
	if(target == UINT32_MAX)
		return ;

	std::list<uint32_t> path =
		get_path(info, mech_cell_position, target, this->range);

	std::vector<uint32_t> vision_indeces(client->get_vision_indeces(info));
	std::sort(vision_indeces.begin(), vision_indeces.end());
	for(auto it = ++path.begin(); it != path.end(); ++it){
		auto curr_cell = info->get_cell(*it);
		if( client->check_previously_visible(info, *it)){
			auto res = std::find(vision_indeces.begin(), vision_indeces.end(), *it);
			if( ((res != vision_indeces.end()) && (curr_cell.unit != nullptr)) ||
				(info->get_cell(*it).is_terrain_obstacle())){
				path.erase(++it, path.end());
				break;
			}
		}
	}

	std::list<uint32_t> aoe =
		torpedo_info_ptr->get_damage_zone(info, path.back()).combine();
	std::list<uint32_t> area = ::area(info, mech_cell_position, this->range).combine();

	for(const uint32_t &cell_index : area){
		client->fill_color_cell(info, cell_index,
			sf::Color(255, 255, 255, 30));
	}

	for(uint32_t &cell_index : path){
		client->fill_color_cell(info, cell_index,
			sf::Color(255, 0, 0));
	}

	for(uint32_t &cell_index : aoe){
		client->fill_color_cell(info, cell_index,
			sf::Color(255, 255, 255, 0), sf::Color(255, 0, 0, 70));
	}
}

std::shared_ptr<torpedo_info>
weapon::torpedo_loading(std::shared_ptr<torpedo_info> torpedo) noexcept{
	auto tmp_torpedo = this->torpedo_info_ptr;
	this->torpedo_info_ptr = torpedo;
	this->curr_delay = 0;
	return tmp_torpedo;
}

void legs::load_sprites(){
	legs::texture.loadFromFile("./../data/legs_button.png");

	legs::sprites["display_off"] = sf::Sprite(legs::texture,
		sf::IntRect(0, 0, 86, 35));
	legs::sprites["display_off"].setPosition(17, 0);

	legs::sprites["display_on"] = sf::Sprite(legs::texture,
		sf::IntRect(0, 35, 86, 35));
	legs::sprites["display_on"].setPosition(17, 0);

	legs::sprites["slow_on"] = sf::Sprite(legs::texture,
		sf::IntRect(86, 0, 35, 35));
	legs::sprites["slow_on"].setPosition(105, 0);

	legs::sprites["slow_off"] = sf::Sprite(legs::texture,
		sf::IntRect(86, 35, 35, 35));
	legs::sprites["slow_off"].setPosition(105, 0);

	legs::sprites["medium_on"] = sf::Sprite(legs::texture,
		sf::IntRect(121, 0, 35, 35));
	legs::sprites["medium_on"].setPosition(142, 0);

	legs::sprites["medium_off"] = sf::Sprite(legs::texture,
		sf::IntRect(121, 35, 35, 35));
	legs::sprites["medium_off"].setPosition(142, 0);

	legs::sprites["fast_on"] = sf::Sprite(legs::texture,
		sf::IntRect(156, 0, 35, 35));
	legs::sprites["fast_on"].setPosition(179, 0);

	legs::sprites["fast_off"] = sf::Sprite(legs::texture,
		sf::IntRect(156, 35, 35, 35));
	legs::sprites["fast_off"].setPosition(179, 0);
};

void engine::load_sprites(){
	engine::texture.loadFromFile("./../data/engine_button.png");

	engine::sprites["picture"] = sf::Sprite(engine::texture,
		sf::IntRect(156, 0, 35, 35));
	engine::sprites["picture"].setPosition(17, 0);

	engine::sprites["threshold_minus_max"] = sf::Sprite(engine::texture,
		sf::IntRect(86, 0, 35, 35));
	engine::sprites["threshold_minus_max"].setPosition(17 + 35 + 2, 0);

	engine::sprites["threshold_minus"] = sf::Sprite(engine::texture,
		sf::IntRect(86, 35, 35, 35));
	engine::sprites["threshold_minus"].setPosition(17 + 35 + 2, 0);

	engine::sprites["display_off"] = sf::Sprite(engine::texture,
		sf::IntRect(0, 0, 86, 35));
	engine::sprites["display_off"].setPosition(91, 0);

	engine::sprites["display_on"] = sf::Sprite(engine::texture,
		sf::IntRect(0, 35, 86, 35));
	engine::sprites["display_on"].setPosition(91, 0);

	engine::sprites["threshold_plus_max"] = sf::Sprite(engine::texture,
		sf::IntRect(121, 0, 35, 35));
	engine::sprites["threshold_plus_max"].setPosition(17 + 35 + 2 + 35 + 2 + 86 + 2, 0);

	engine::sprites["threshold_plus"] = sf::Sprite(engine::texture,
		sf::IntRect(121, 35, 35, 35));
	engine::sprites["threshold_plus"].setPosition(17 + 35 + 2 + 35 + 2 + 86 + 2, 0);
};

engine::engine(uint32_t id, deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, std::string name,
	float weight, uint32_t slots, int threshold_ = 0)
	: item(id, part_ptr, name, weight, slots), performance{60.0f, 60.0f, 0.3f},
		threshold(threshold_),
		threshold_text_ptr(
			create_text(text_delete_contaier, std::string("threshold"), get_font(), 20)),
		threshold_value_text_ptr(
			create_text(text_delete_contaier, std::to_string(threshold_), get_font(), 21)){

	if(engine::sprites.empty())
		engine::load_sprites();
}

mech_status engine::get_mech_changes(float time, const mech_status &status) const noexcept{
	float fuel = status.current_fuel;
	float current_energy_rate = status.current_energy / status.energy_capacity;
	float necessary_energy_rate = (float)this->threshold / 100.f;

	const auto &perf = this->performance;

	float rate = (fuel - (perf.spend_fuel * time)) < 0 ? fuel / perf.spend_fuel * time : 1.0f;
	return (current_energy_rate < necessary_energy_rate) && (fuel > 0) &&
		(this->get_power_status()) ?
		mech_status::current(perf.receive_energy, perf.receive_heat, -perf.spend_fuel)
		* time * rate :
		mech_status::zero();
}

item_shape engine::get_draw_shape(const mech* owner, client *client,
	const sf::Vector2f& position){

	float scale = client->get_view_scale();
	set_scale_f(scale, engine::sprites);

	item_shape shape{};
	shape.elements.emplace_back(engine::sprites["picture"], std::function<void()>());
	if(this->get_power_status()){
		shape.elements.emplace_back(engine::sprites["display_on"], std::function<void()>());

		shape.text_elements.emplace_back(update_text_f(
			scale, sf::Vector2f(95, 3), this->threshold_text_ptr.get(), sf::Color(112, 166, 65)));

		this->threshold_value_text_ptr->setString(
			std::to_string(this->threshold) + std::string("%"));
		shape.text_elements.emplace_back(update_text_f(scale, sf::Vector2f(115, -11),
			this->threshold_value_text_ptr.get(), sf::Color(112, 166, 65)));
	} else {
		shape.elements.emplace_back(engine::sprites["display_off"], std::function<void()>());
	}

	if(this->threshold == 0){
		shape.elements.emplace_back(engine::sprites["threshold_minus_max"], std::function<void()>());
	} else {
		shape.elements.emplace_back(engine::sprites["threshold_minus"],
			this->get_power_status() ? [this](){this->add_threshold(-10);} : std::function<void()>());
	}

	if(this->threshold == 100){
		shape.elements.emplace_back(engine::sprites["threshold_plus_max"], std::function<void()>());
	} else {
		shape.elements.emplace_back(engine::sprites["threshold_plus"],
			this->get_power_status() ? [this](){this->add_threshold(10);} : std::function<void()>());
	}

	place_shape_f(shape, scale, position);
	shape += turn_on::get_draw_shape(owner, client, position);
	return shape;
}

legs::legs(uint32_t id, deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, std::string name, float weight, uint32_t slots)
	: item(id, part_ptr, name, weight, slots),
	modes{{0.3f, -2, 1}, {1.0f, -10, 5}, {3.0f, -70, 10}}{

	if(legs::sprites.empty())
		legs::load_sprites();

	this->speed[(int)terrain_en::RIVER] = 0;
	this->speed[(int)terrain_en::MOUNTAIN] = 0.5;
	this->speed[(int)terrain_en::PLAIN] = 2;
	this->speed[(int)terrain_en::PALM] = 2;
}

item_shape legs::get_draw_shape(const mech* owner, client *client,
	const sf::Vector2f& position){

	float scale = client->get_view_scale();
	set_scale_f(scale, legs::sprites);

	item_shape shape{};
	if(this->get_power_status()){
		shape.elements.emplace_back(legs::sprites["display_on"], std::function<void()>());
	} else {
		shape.elements.emplace_back(legs::sprites["display_off"], std::function<void()>());
	}
	if(this->current_mode == legs::mode_name::slow){
		shape.elements.emplace_back(legs::sprites["slow_on"], std::function<void()>());
	} else {
		shape.elements.emplace_back(legs::sprites["slow_off"],
			[this](){this->set_mode(legs::mode_name::slow);});
	}
	if(this->current_mode == legs::mode_name::medium){
		shape.elements.emplace_back(legs::sprites["medium_on"], std::function<void()>());
	} else {
		shape.elements.emplace_back(legs::sprites["medium_off"],
			[this](){this->set_mode(legs::mode_name::medium);});
	}
	if(this->current_mode == legs::mode_name::fast){
		shape.elements.emplace_back(legs::sprites["fast_on"], std::function<void()>());
	} else {
		shape.elements.emplace_back(legs::sprites["fast_off"],
			[this](){this->set_mode(legs::mode_name::fast);});
	}

	place_shape_f(shape, scale, position);
	shape += turn_on::get_draw_shape(owner, client, position);
	return shape;
}


mech_status legs::get_mech_changes_legs(float time) const noexcept{
	const float& energy = this->modes[(int)current_mode].energy;
	const float& heat = this->modes[(int)current_mode].heat;
	return mech_status::current(energy, heat) * time;
};

mech_status change_mech_status::get_mech_changes(
	float time, const mech_status &status) const noexcept{
	return mech_status::zero();
}

void cooling_system::load_sprites(){
	cooling_system::texture.loadFromFile("./../data/cooling_system.png");

	cooling_system::sprites["picture"] = sf::Sprite(cooling_system::texture,
		sf::IntRect(0, 0, 35, 35));
	cooling_system::sprites["picture"].setPosition(17, 0);
}

cooling_system::cooling_system(uint32_t id, deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, std::string name_, float weight, uint32_t slots,
	float heat_efficiency_, float energy_consumption_)
	: item(id, part_ptr, name_, weight, slots), heat_efficiency(heat_efficiency_),
	energy_consumption(energy_consumption_){
	if(cooling_system::sprites.empty())
		cooling_system::load_sprites();
}

mech_status cooling_system::get_mech_changes(
	float time, const mech_status &status) const noexcept{

	return (this->get_power_status()) && (status.current_heat > 40) ?
		mech_status::current(-energy_consumption * time, -heat_efficiency * time) :
		mech_status::zero();
}

item_shape cooling_system::get_draw_shape(const mech* owner, client *client,
	const sf::Vector2f& position){

		float scale = client->get_view_scale();
		set_scale_f(scale, cooling_system::sprites);

		item_shape shape{};
		shape.elements.emplace_back(cooling_system::sprites["picture"],std::function<void()>());

		place_shape_f(shape, scale, position);
		shape += turn_on::get_draw_shape(owner, client, position);
		return shape;

}

accumulator::accumulator(uint32_t id, deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, std::string name,
	float weight, uint32_t slots, float capacity_)
	: item(id, part_ptr, name, weight, slots), capacity(capacity_){
};

mech_status accumulator::get() const noexcept{
	return mech_status::capacity(this->capacity, 0, 0);
}

radiator::radiator(uint32_t id, deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, std::string name,
	float weight, uint32_t slots, float capacity_)
	: item(id, part_ptr, name, weight, slots), capacity(capacity_){
};

mech_status radiator::get() const noexcept{
	return mech_status::capacity(0, this->capacity, 0);
}

tank::tank(uint32_t id, deferred_deletion_container<sf::Text> *text_delete_contaier,
	const part_of_mech *part_ptr, std::string name,
	float weight, uint32_t slots, float capacity_)
	: item(id, part_ptr, name, weight, slots), capacity(capacity_){
};

mech_status tank::get() const noexcept{
	return mech_status::capacity(0, 0, this->capacity);
}
