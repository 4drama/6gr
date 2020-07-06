#include "unit.hpp"

#include <cmath>
#include <iostream>
#include <cassert>

std::map<std::string, sf::Texture> unit::textures{};

sf::Texture projectile::texture{};
std::map<std::string, sf::Sprite> projectile::sprites{};

void projectile::load_sprites(){
	projectile::texture.loadFromFile("./../data/torpedo.png");

	sprites["torpedo"] = sf::Sprite(projectile::texture,
		sf::IntRect(0, 0, 41, 14));
	sprites["torpedo"].setPosition(0, 0);
}

projectile::projectile(std::list<uint32_t> path_, uint32_t cell_index_, uint32_t aoe_)
	: path(path_), cell_index(cell_index_), aoe(aoe_){
	this->path.pop_front();
	if(projectile::sprites.empty())
		projectile::load_sprites();
}

void projectile::update(game_info *info, float time){
	assert(path.size() != 0);

	this->path_progress +=  this->speed * (time / 10000);

	if(this->path_progress > 1){
		this->cell_index = this->path.front();
		this->path.pop_front();
		this->path_progress -= 1;
//		this->lifetime -= 1;

		if( this->path.empty() || info->get_cell(this->cell_index).is_soft_obstacle()){
			this->explosion = true;
		}
	}
}

void unit::update_path(game_info *info, uint32_t player_index, uint32_t target_cell){
	auto old_front_cell = this->path.front();

	this->path = path_find(info, this->cell_index,
		target_cell, shared_from_this(), player_index, true);

	if(old_front_cell != this->path.front())
		this->path_progress = 0;
}

bool unit::event(sf::Event event, game_info *info, uint32_t player_index,
	uint32_t target_cell) noexcept{

	if(event.type == sf::Event::MouseButtonPressed &&
		event.mouseButton.button == sf::Mouse::Button::Right){

		this->update_path(info, player_index, target_cell);
		return true;
	}
	return false;
}

item_button::item_button(sf::Sprite sprite_, std::function<void()> func_)
	: sprite(sprite_), func(func_) {
};

namespace{

sf::Sprite create_sprite_f(sf::Texture *texture,
	int width, int height, int column, int raw){

	sf::Sprite frame;

	sf::IntRect rectangle{0 + width * column, 0 + height * raw,
		 width, height};

	frame.setTexture(*texture);
	frame.setTextureRect(rectangle);
	frame.setOrigin(width / 2, height);
	frame.setScale (0.2, -0.2);
	return frame;
}

}

unit::unit(uint32_t cell_index_, uint32_t vision_range_)
	: cell_index(cell_index_), vision_range(vision_range_){
}

mech::mech(uint32_t cell_index_)
	: unit(cell_index_, 4),
	left_arm(60, 25, 7, 2.0f),
	torso(120, 80, 18, 1.0f),
	right_arm(60, 25, 7, 2.0f),
	energy_text("energy_value", get_font(), 22),
	heat_text("heat_value", get_font(), 22),
	fuel_text("fuel_text", get_font(), 22){

	std::string path("./../data/");
	std::string filename("mech_60x60x6.png");
	unit::textures[filename].loadFromFile(path + filename);

	this->sprites.emplace_back(
		create_sprite_f(&unit::textures[filename],
		60, 60, 0, 0));

	this->left_arm.add_item(std::make_shared<weapon>("Rocket", 15000));
	this->left_arm.add_item(std::make_shared<radiator>("Radiator", 75));
	this->left_arm.add_item(std::make_shared<accumulator>("Accumulator", 50));

	this->right_arm.add_item(std::make_shared<weapon>("Rocket", 15000));
	this->right_arm.add_item(std::make_shared<radiator>("Radiator", 75));
	this->right_arm.add_item(std::make_shared<accumulator>("Accumulator", 50));

	this->torso.add_item(std::make_shared<legs>("Legs"));
	this->torso.add_item(std::make_shared<engine>("Engine", 70));
	this->torso.add_item(std::make_shared<tank>("Tank", 20));
	this->torso.add_item(std::make_shared<radiator>("Radiator", 200));
	this->torso.add_item(std::make_shared<accumulator>("Accumulator", 100));
	this->torso.add_item(std::make_shared<cooling_system>("Cooling system", 10.0f, 1.25f));

	this->refresh();
	this->calculate_status(mech_status::current(0, 0, 20));
}

float mech::get_speed(terrain_en ter_type) const noexcept{
	return legs_ptr ? legs_ptr->get_speed(ter_type) : 0;
};

part_of_mech::part_of_mech(float durability_,
	float weight_, uint32_t slots_, float priority_)
	: durability(durability_), max_durability(durability_),
	status(mech_status::zero()), priority(priority_), limits{weight_, slots_},
	durability_text("durability_value", get_font(), 22){
}

void part_of_mech::prepare_for_refresh() noexcept{
	this->status.clear_capacity();

	this->weight = 0;
	this->slots = 0;
}

bool part_of_mech::add_item(std::shared_ptr<item> item){
	if(((this->weight + item->get_weight()) <= this->limits.weight) &&
		((this->slots + item->get_slots()) <= this->limits.slots)){

		this->items.emplace_back(item);
		this->weight += item->get_weight();
		this->slots += item->get_slots();
		return true;
	} else {
		std::cerr << "Can't add item: " << item->get_name() << ". "
			<< "Weight: " << this->weight << '+' << item->get_weight() << '/'
			<< this->limits.weight << ". Slots: " << this->slots << '+' << item->get_slots()
			<< '/' << this->limits.slots << std::endl;
		return false;
	}
};

void part_of_mech::validate(){
	if(this->durability <= 0){
		this->status.current_energy = 0;
		this->status.current_fuel = 0;
	}
}

void mech::refresh(){
	this->legs_ptr = nullptr;
	this->engine_ptr = nullptr;

	left_arm.prepare_for_refresh();
	right_arm.prepare_for_refresh();
	torso.prepare_for_refresh();

	for(auto& item : this->left_arm.items){
		if(item->is_capacity_change()){
			capacity_change* item_ptr = item->is_capacity_change();
			this->left_arm.status += item_ptr->get();
		}
		this->left_arm.weight += item->get_weight();
		this->left_arm.slots += item->get_slots();
	}

	for(auto& item : this->right_arm.items){
		if(item->is_capacity_change()){
			capacity_change* item_ptr = item->is_capacity_change();
			this->right_arm.status += item_ptr->get();
		}
		this->right_arm.weight += item->get_weight();
		this->right_arm.slots += item->get_slots();
	}

	for(auto& item : this->torso.items){
		if(item->is_legs()){
			this->legs_ptr = item->is_legs();
		}
		if(item->is_engine()){
			this->engine_ptr = item->is_engine();
		}
		if(item->is_capacity_change()){
			capacity_change* item_ptr = item->is_capacity_change();
			this->torso.status += item_ptr->get();
		}
		this->torso.weight += item->get_weight();
		this->torso.slots += item->get_slots();
	}
}

item_shape mech::get_status_shape(client *client, const sf::Vector2f& position) const{
	float scale = client->get_view_scale();

	sf::Vector2f corner = sf::Vector2f(
		-client->get_view_width(), client->get_view_height()) / 2.0f;


	item_shape shape{};
	auto add_rectangle = [&scale, &shape, &corner](sf::Vector2f size,
		sf::Vector2f offset, sf::Vector2f pos, sf::Color color = sf::Color::White){

		sf::RectangleShape rect(size * scale);
		rect.setPosition(corner + (offset + pos) * scale);
		rect.setFillColor(color);
		shape.bar_elements.emplace_back(rect);
	};

	auto add_text = [&scale, &shape, &corner](sf::Text *text, sf::Vector2f offset,
		sf::Vector2f pos, std::string str, sf::Color color = sf::Color::White){

		text->setScale(scale, -scale);
		text->setPosition(corner + (offset + pos) * scale);
		text->setColor(color);
		text->setString(str);
		shape.text_elements.emplace_back(text);
	};

	auto add_bar = [&scale, &shape, &corner, &add_rectangle, &add_text](
		sf::Vector2f offset, float max_value, float curr_value,
		sf::Text *text_value, sf::Color value_color, float threshold = 0){

		add_rectangle(sf::Vector2f(50, 300), offset, sf::Vector2f(0, 0),
			sf::Color(80, 100, 125));
		add_rectangle(sf::Vector2f(50, 300 * (curr_value / max_value)), offset,
			sf::Vector2f(0, 0),	value_color);

		if(threshold != 0){
			add_rectangle(sf::Vector2f(50, 3), offset,
				sf::Vector2f(0, 298 * threshold), sf::Color(255, 0, 0));
		}

		add_rectangle(sf::Vector2f(2, 300), offset, sf::Vector2f(50, 0));
		add_rectangle(sf::Vector2f(65, 2), offset, sf::Vector2f(0, 300));
		add_rectangle(sf::Vector2f(52, 2), offset, sf::Vector2f(0, -2));

		for(float point = -1; point < -1 + 60 * 6; point += 60){
			add_rectangle(sf::Vector2f(16, 2), offset, sf::Vector2f(34, point));
		}

		for(float point = 30; point < 30 + 60 * 5; point += 60){
			add_rectangle(sf::Vector2f(8, 2), offset, sf::Vector2f(42, point));
		}

		add_text(text_value, offset, sf::Vector2f(20, 330),
			std::to_string((int)curr_value));
	};

	auto status = this->accumulate_status();

	add_bar(sf::Vector2f(20, 20), status.energy_capacity, status.current_energy,
		 &this->energy_text, sf::Color(111, 189, 80),
		 (engine_ptr && engine_ptr->get_power_status()) ? engine_ptr->get_threshold() : 0);

	add_bar(sf::Vector2f(90, 20), status.heat_capacity, status.current_heat,
 		 &this->heat_text, sf::Color(136, 52, 14));

	if(status.fuel_capacity != 0){
		add_bar(sf::Vector2f(160, 20), status.fuel_capacity, status.current_fuel,
	 		 &this->fuel_text, sf::Color(55, 49, 29));
	};

	auto add_capacity_rectangle = [&scale, &shape](sf::Vector2f size,
		sf::Vector2f offset, sf::Vector2f pos, sf::Color color = sf::Color::White){

		sf::RectangleShape rect(size * scale);
		rect.setPosition((offset + pos) * scale);
		rect.setFillColor(color);
		shape.bar_elements.emplace_back(rect);
	};

	auto add_capacity_text = [&scale, &shape](sf::Text *text, sf::Vector2f offset,
		sf::Vector2f pos, std::string str, sf::Color color = sf::Color::White){

		text->setScale(scale, -scale);
		text->setPosition((offset + pos) * scale);
		text->setColor(color);
		text->setString(str);
		text->setOutlineThickness(1.0f);
		text->setOutlineColor(sf::Color(50, 50, 200));
		shape.text_elements.emplace_back(text);
	};

	auto add_capacity_bar = [&scale, &shape,
		&add_capacity_rectangle, &add_capacity_text](
		sf::Vector2f offset, const part_of_mech& part){

		float rate = part.durability / part.max_durability;

		add_capacity_rectangle(sf::Vector2f(227, 35), offset, sf::Vector2f(0, 0),
			part.durability > 0 ? sf::Color(80, 100, 125) : sf::Color(10, 30, 55));
		add_capacity_rectangle(sf::Vector2f(227 * rate, 35), offset, sf::Vector2f(0, 0),
			sf::Color( (160 * (1 - rate)) + (std::sin(rate * 3.14) * 120),
			(160 * rate) + (std::sin(rate * 3.14) * 120), 60));
		add_capacity_text(&part.durability_text, offset, sf::Vector2f(5, 33),
			std::to_string((int)part.durability) +
			"/" + std::to_string((int)part.max_durability));
	};

	add_capacity_bar(sf::Vector2f{-113, -435}, this->torso);
	add_capacity_bar(sf::Vector2f{-413, -435}, this->left_arm);
	add_capacity_bar(sf::Vector2f{187, -435}, this->right_arm);

	auto zone_debug = [&add_rectangle](sf::Vector2f offset, const mech_status &stat){
		add_rectangle(sf::Vector2f(20, stat.current_energy / 3),
			offset, sf::Vector2f(0, 0),
			stat.current_energy <= stat.energy_capacity ?
			sf::Color(0, 255, 0) : sf::Color(255, 255, 255));

		add_rectangle(sf::Vector2f(20, -stat.current_heat / 3),
			offset, sf::Vector2f(0, 0),
			stat.current_heat <= stat.heat_capacity ?
			sf::Color(255, 0, 0) : sf::Color(255, 255, 255));
	};

	zone_debug(sf::Vector2f(20, 500), this->left_arm.status);
	zone_debug(sf::Vector2f(50, 520), this->torso.status);
	zone_debug(sf::Vector2f(80, 500), this->right_arm.status);

	return shape;
}

item_shape mech::prepare_shape(client *client) const{
	item_shape item_shape{};
	auto get_zone_shape = [this, client, &item_shape](auto &zone, sf::Vector2f pos){
		sf::Vector2f offset(0, 40);
		uint32_t counter = 0;
		for(auto &item : zone){
			auto tmp_shape = item->get_draw_shape(this, client,
				pos + offset * (float)counter);
			if(tmp_shape){
				item_shape += tmp_shape;
				++counter;
			}
		}
	};
	get_zone_shape(this->torso.items, sf::Vector2f{-100, -360});
	get_zone_shape(this->left_arm.items, sf::Vector2f{-400, -360});
	get_zone_shape(this->right_arm.items, sf::Vector2f{200, -360});

	return item_shape;
}

void mech::draw_gui(game_info *info, client *client){
	item_shape item_shape = prepare_shape(client);
	auto status_shape = this->get_status_shape(client, sf::Vector2f{0, 0});
	client->draw_item_shape(item_shape);
	client->draw_item_shape(status_shape);

	if((waiting_confirm != nullptr) && waiting_confirm->is_path_draw()){
		path_draw *item_draw_ptr = waiting_confirm->is_path_draw();
		item_draw_ptr->draw_active_zone(this->cell_index, info, client);
	}
}

bool mech::interact_gui(game_info *info, client *client){
	float scale = client->get_view_scale();
	sf::Vector2f pos = client->mouse_on_map();
	item_shape shape = prepare_shape(client);
	for(auto &but : shape.elements){
		if(is_inside(but.sprite, pos)){
			if(but.func){
				but.func();
			}
			return true;
		}
	}
	return false;
}

bool mech::event(sf::Event event, game_info *info, uint32_t player_index,
	uint32_t target_cell) noexcept{

	if(event.type == sf::Event::MouseButtonPressed){
		switch(event.mouseButton.button){
			case sf::Mouse::Button::Left :
				if(this->waiting_confirm != nullptr){
					this->waiting_confirm->is_weapon()->use(info, this, target_cell);
					this->waiting_confirm = nullptr;
					return true;
				}
				return false;
			case sf::Mouse::Button::Right :
				if(this->waiting_confirm == nullptr){
					this->update_path(info, player_index, target_cell);
					return true;
				} else {
					this->waiting_confirm = nullptr;
					return true;
				}
			default :
				return false;
		}
	}
	return false;
}

mech_status mech::accumulate_status() const noexcept{
	mech_status status = mech_status::zero();
	auto add_part = [&status](const part_of_mech &part){
		if(part.durability > 0)
			status += part.status;
	};

	add_part(this->left_arm);
	add_part(this->torso);
	add_part(this->right_arm);
	return status;
}

namespace{

std::vector<std::vector<part_of_mech*>> calculate_levels_f(
	std::vector<part_of_mech*> parts, const std::pair<mech_status::type, float> &val){
	using parts_container_type = std::vector<part_of_mech*>;

	auto to_remove = [&val](part_of_mech* part){
		return ((part->durability <= 0.0f) ||
		( is_store(val) ? (part->status.is_full(val.first)) :
		!(part->status.is_useful(val.first, val.second))));
	};
	parts.erase(std::remove_if(parts.begin(), parts.end(), to_remove), parts.end());

	auto more_priority = [](const part_of_mech* left, const part_of_mech* right){
		return left->priority > right->priority;
	};
	std::sort(parts.begin(), parts.end(), more_priority);

	std::vector<parts_container_type> priority_levels{};

	for(auto it = parts.begin(); it != parts.end(); ){
		auto if_less_then_it = [it](const part_of_mech* part){
			return part->priority < (*it)->priority;
		};
		auto end = std::find_if(it, parts.end(), if_less_then_it);
		priority_levels.emplace_back(it, end);
		it = end;
	}
	return priority_levels;
}

}

void mech::calculate_status(const mech_status &status){
	using parts_container_type = std::vector<part_of_mech*>;
	using priority_levels_type = std::vector<parts_container_type>;

	auto parts_of_status = status.get();
	parts_container_type parts_of_mech{&this->left_arm, &this->torso, &this->right_arm};

	std::map<mech_status::type, priority_levels_type> priorities_by_type{};
	for(auto &stat : parts_of_status){
		priorities_by_type[stat.first] = calculate_levels_f(parts_of_mech, stat);
	}

	mech_status rest = mech_status::zero();
	for(auto &part_stat : parts_of_status){
		const priority_levels_type *curr_priority_ptr = &priorities_by_type[part_stat.first];
		if(curr_priority_ptr->empty())
			continue;

		const parts_container_type *curr_parts_ptr = is_store(part_stat) ?
			&priorities_by_type[part_stat.first].back() :
			&priorities_by_type[part_stat.first].front();

		std::pair<mech_status::type, float> div_part_stat = {
			part_stat.first, part_stat.second / (uint32_t)curr_parts_ptr->size() };

		for(auto &priority_part : *curr_parts_ptr){
			rest.add_current(priority_part->status.try_spend(div_part_stat));
		}
	}
	if(rest){
		this->calculate_status(rest);
	}
}

float mech::get_available_rate(mech_status necessary) const noexcept{
	const mech_status status = this->accumulate_status();
	const float &energy_available = status.current_energy;
	float heat_available = status.heat_capacity - status.current_heat;

	float energy_necessary = -necessary.current_energy;
	const float& heat_necessary = necessary.current_heat;

	float rate = 1;
	if((energy_available < energy_necessary) || (heat_available < heat_necessary)){
		float energy_rate = energy_available / -energy_necessary;
		float heat_rate = heat_available / heat_necessary;
		rate = energy_rate < heat_rate ? energy_rate : heat_rate;
	}
	return rate > 0 ? rate : 0;
}

float mech::move_calculate(float time, terrain_en ter_type) noexcept{
	if(!this->legs_ptr)
		return 0;
	if(!this->legs_ptr->get_power_status())
		return 0;

	mech_status diff = this->legs_ptr->get_mech_changes_legs(time / 10000);
	float rate = this->get_available_rate(diff);

	this->calculate_status(diff * rate);
	return this->get_speed(ter_type) * time * rate;
}

void unit::open_vision(game_info *info, uint32_t player_index){
	this->vision_indeces = open_adjacent(info, player_index,
		this->cell_index, this->vision_range);
}

void unit::unit_update_move(game_info *info, uint32_t player_index, float time){
	if(!this->path.size())
		return;

	terrain_en terr_type = info->get_cell(this->path.front()).ter.type;
	this->path_progress += move_calculate(time, terr_type);

	if((this->cell_index == this->path.back())
		|| (info->get_cell(this->path.front()).unit != nullptr) ){
		this->path.clear();
	}

	if(this->path_progress > 1){
		info->map[this->cell_index].unit = nullptr;
		this->cell_index = this->path.front();
		info->map[this->cell_index].unit = shared_from_this();
		this->path.pop_front();

		this->path_progress -= 1;

		if(!this->path.empty()){
			uint32_t recalculated_depth = 8;
			std::list<uint32_t> recalculated_path;

			auto it = this->path.begin();
			if(this->path.size() <= recalculated_depth)
				it = --this->path.end();
			else
				std::advance(it, recalculated_depth);

			recalculated_path = path_find(info, this->cell_index,
				*it, shared_from_this(), player_index, false);

			recalculated_path.splice(recalculated_path.end(),
				this->path, it, this->path.end());

			this->path = recalculated_path;
		}
		this->open_vision(info, player_index);
	}
}


void unit::update(game_info *info, uint32_t player_index, float time){
	this->unit_update_move(info, player_index, time);
	this->update_v(info, player_index, time);
}

void mech::update_v(game_info *info, uint32_t player_index, float time){
	auto update = [this, &time](auto &zone){
		zone.validate();
		for(auto &item : zone.items){
			if(item->is_change_mech_status()){
				auto change_item = item->is_change_mech_status();
				mech_status diff = change_item->get_mech_changes(time / 10000,
					this->accumulate_status());
				float rate = this->get_available_rate(diff);
				this->calculate_status(diff * rate);
			}

			item->update(this, time);
		}
	};
	update(this->torso);
	update(this->left_arm);
	update(this->right_arm);
}

std::pair<mech_status::type, float>
mech_status::try_spend(const std::pair<mech_status::type, float> &val){
	float rest = 0;
	auto less_then_zero = [&rest](float &current_status){
		if(current_status < 0){
			rest = current_status;
			current_status = 0;
		}
	};

	auto more_then_value = [&rest](float &current_status, float &value){
		if(current_status > value){
			rest = current_status - value;
			current_status = value;
		}
	};

	switch(val.first){
		case mech_status::type::energy :
			this->current_energy += val.second;
			less_then_zero(this->current_energy);
			more_then_value(this->current_energy, this->energy_capacity);
		break;

		case mech_status::type::heat :
			this->current_heat += val.second;
			less_then_zero(this->current_heat);
			more_then_value(this->current_heat, this->heat_capacity);
		break;

		case mech_status::type::fuel :
			this->current_fuel += val.second;
			less_then_zero(this->current_fuel);
			more_then_value(this->current_fuel, this->fuel_capacity);
		break;
	}
	return {val.first, rest};
}

mech_status& mech_status::add_current(const std::pair<mech_status::type, float> &val){
	switch(val.first){
		case mech_status::type::energy : this->current_energy += val.second;
		break;

		case mech_status::type::heat : this->current_heat += val.second;
		break;

		case mech_status::type::fuel : this->current_fuel += val.second;
		break;
	}
	return *this;
}

mech_status mech_status::try_spend(const mech_status& right){
	mech_status rest = mech_status::zero();
	auto parts = right.get();
	for(auto &part : parts){
		auto part_rest = try_spend(part);
		rest.add_current(part_rest);
	}
	return rest;
};

bool mech_status::is_useful(mech_status::type type, float value) const noexcept{
	switch(type){
		case mech_status::type::energy :
			return value < 0 ? (this->current_energy > 0.0f) : true;

		case mech_status::type::heat :
			return value > 0 ? (this->current_heat < this->heat_capacity) : true;

		case mech_status::type::fuel :
			return value < 0 ? (this->current_fuel > 0.0f) : true;
		default:
			return false;
	}
}

bool mech_status::is_full(mech_status::type type) const noexcept{
	switch(type){
		case mech_status::type::energy :
			return this->current_energy >= this->energy_capacity ? true : false;

		case mech_status::type::heat :
			return this->current_heat <= 0 ? true : false;

		case mech_status::type::fuel :
			return this->current_fuel >= this->fuel_capacity ? true : false;
		default:
			return false;
	}
}

bool is_store(const std::pair<mech_status::type, float> &val){
	switch(val.first){
		case mech_status::type::energy :
			return val.second > 0 ? true : false;

		case mech_status::type::heat :
			return val.second < 0 ? true : false;

		case mech_status::type::fuel :
			return val.second > 0 ? true : false;
		default:
			return false;
	}
}
