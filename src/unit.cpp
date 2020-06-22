#include "unit.hpp"

#include <iostream>

std::map<std::string, sf::Texture> unit::textures{};

item_button::item_button(sf::Sprite sprite_, std::function<void()> func_)
	: sprite(sprite_), func(func_) {
};

bool is_inside_sprite(sf::Sprite sprite, sf::Vector2f pos){
	sf::FloatRect rect = sprite.getGlobalBounds();

	if((rect.left <= pos.x) && ((rect.left + rect.width) >= pos.x) &&
		(rect.top <= pos.y) && ((rect.top + rect.height) >= pos.y))
		return true;
	else
		return false;
}

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
	left_arm{60, mech_status{0, 70, 0, 50, 20, 20}, std::list<std::shared_ptr<item>>{}, 2.0f},
	torso{60, mech_status{0, 100, 0, 50, 0, 0}, std::list<std::shared_ptr<item>>{}, 1.0f},
	right_arm{60, mech_status{0, 0, 0, 50, 0, 0}, std::list<std::shared_ptr<item>>{}, 2.0f},
	energy_text("energy_value", get_font(), 22),
	heat_text("heat_value", get_font(), 22),
	fuel_text("fuel_text", get_font(), 22){

	std::string path("./../data/");
	std::string filename("mech_60x60x6.png");
	unit::textures[filename].loadFromFile(path + filename);

	this->sprites.emplace_back(
		create_sprite_f(&unit::textures[filename],
		60, 60, 0, 0));

	this->left_arm.items.emplace_back(std::make_shared<item>("Rocket", 15000));
	this->right_arm.items.emplace_back(std::make_shared<item>("Rocket", 15000));
	this->torso.items.emplace_back(std::make_shared<legs>("Legs"));
	this->torso.items.emplace_back(std::make_shared<engine>("Engine", 70));
	this->refresh();
}

float mech::get_speed(terrain_en ter_type) const noexcept{
	return legs_ptr ? legs_ptr->get_speed(ter_type) : 0;
};

void mech::refresh(){
	this->legs_ptr = nullptr;
	this->engine_ptr = nullptr;
	for(auto& item : this->left_arm.items){

	}

	for(auto& item : this->right_arm.items){

	}

	for(auto& item : this->torso.items){
		if(item->is_legs()){
			this->legs_ptr = item->is_legs();
		}
		if(item->is_engine()){
			this->engine_ptr = item->is_engine();
		}
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

	return shape;
}

item_shape mech::prepare_shape(client *client) const{
	item_shape item_shape{};
	auto get_zone_shape = [this, client, &item_shape](auto &zone, sf::Vector2f pos){
		sf::Vector2f offset(0, 40);
		uint32_t counter = 0;
		for(auto &item : zone){
			item_shape += item->get_draw_shape(this, client,
				pos + offset * (float)counter++);
		}
	};
	get_zone_shape(this->torso.items, sf::Vector2f{-100, -200});
	get_zone_shape(this->left_arm.items, sf::Vector2f{-400, -200});
	get_zone_shape(this->right_arm.items, sf::Vector2f{200, -200});

	return item_shape;
}

void mech::draw_gui(game_info *info, client *client){
	item_shape item_shape = prepare_shape(client);
	auto status_shape = this->get_status_shape(client, sf::Vector2f{0, 0});
	client->draw_item_shape(item_shape);
	client->draw_item_shape(status_shape);
}

bool mech::interact_gui(game_info *info, client *client){
	float scale = client->get_view_scale();
	sf::Vector2f pos = client->mouse_on_map();
	item_shape shape = prepare_shape(client);
	for(auto &but : shape.elements){
		if(is_inside_sprite(but.sprite, pos)){
			if(but.func){
				but.func();
			}
			return true;
		}
	}
	return false;
}

mech_status mech::accumulate_status() const noexcept{
	mech_status status = mech_status::zero();
	auto add_part = [&status](const part_of_mech &part){
		if(part.durability != 0)
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
		return ((part->durability <= 0.0f) || !(part->status.is_useful(val.first, val.second)));
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
		std::pair<mech_status::type, float> div_part_stat = {
			part_stat.first, part_stat.second /
			(uint32_t)priorities_by_type[part_stat.first].front().size() };

		for(auto &priority_part : priorities_by_type[part_stat.first].front()){
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
	auto item_update = [this, &time](auto &zone){
		for(auto &item : zone){
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
	item_update(this->torso.items);
	item_update(this->left_arm.items);
	item_update(this->right_arm.items);
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
		break;

		case mech_status::type::heat :
			this->current_heat += val.second;
			more_then_value(this->current_heat, this->heat_capacity);
		break;

		case mech_status::type::fuel :
			this->current_fuel += val.second;
			less_then_zero(this->current_energy);
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
