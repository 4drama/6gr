#include "unit.hpp"

#include <iostream>

sf::Texture item::texture{};
std::map<std::string, sf::Sprite> item::sprites{};

sf::Texture legs::texture{};
std::map<std::string, sf::Sprite> legs::sprites{};

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

void item::load_sprites(){
	item::texture.loadFromFile("./../data/item_button.png");

	sprites["active_button"] = sf::Sprite(item::texture,
		sf::IntRect(0, 0, 180, 23));
	sprites["active_button"].setPosition(34, 0);

	sprites["inactive_button"] = sf::Sprite(item::texture,
		sf::IntRect(0, 23, 180, 23));
	sprites["inactive_button"].setPosition(34, 0);

	sprites["power_off"] = sf::Sprite(item::texture,
		sf::IntRect(197, 23, 28, 35));
	sprites["power_off"].setPosition(-13, 0);

	sprites["power_on"] = sf::Sprite(item::texture,
		sf::IntRect(225, 23, 28, 35));
	sprites["power_on"].setPosition(-13, 0);

	sprites["active_hotkey_screen"] = sf::Sprite(item::texture,
		sf::IntRect(180, 0, 15, 23));
	sprites["active_hotkey_screen"].setPosition(17, 0);

	sprites["inactive_hotkey_screen"] = sf::Sprite(item::texture,
		sf::IntRect(180, 23, 15, 23));
	sprites["inactive_hotkey_screen"].setPosition(17, 0);

	sprites["active_delay_screen"] = sf::Sprite(item::texture,
		sf::IntRect(0, 46, 197, 10));
	sprites["active_delay_screen"].setPosition(17, -25);

	sprites["inactive_delay_screen"] = sf::Sprite(item::texture,
		sf::IntRect(0, 56, 197, 10));
	sprites["inactive_delay_screen"].setPosition(17, -25);

	sprites["progress_bar_ready"] = sf::Sprite(item::texture,
		sf::IntRect(195, 0, 35, 6));
	sprites["progress_bar_ready"].setPosition(22, -27);

	sprites["progress_bar_not_ready"] = sf::Sprite(item::texture,
		sf::IntRect(195, 6, 35, 6));
	sprites["progress_bar_not_ready"].setPosition(22, -27);

	sprites["progress_bar_reload_ready"] = sf::Sprite(item::texture,
		sf::IntRect(195, 12, 35, 6));
	sprites["progress_bar_reload_ready"].setPosition(22, -27);
}

item::item(std::string name_, float delay_)
	: name(name_), delay(delay_), name_text(name, get_font(), 22){
	if(item::sprites.empty())
		item::load_sprites();
	this->name_text.setPosition(39, 5);
}

bool item::get_ready(const mech* owner) const noexcept{
	bool status = false;
	if(this->get_power_status() && (this->get_delay() >= 1) && this->has_resources(owner)){
		status = true;
	}
	return status;
}

void item::update(mech* owner, float time){
	if(this->curr_delay < this->delay){
		this->curr_delay += time;
	}
}

item_shape item::get_draw_shape(const mech* owner, client *client,
	const sf::Vector2f& position){
	float scale = client->get_view_scale();

	for(auto &sprite : item::sprites){
		sprite.second.setScale(scale, -scale);
	}

	item_shape shape{};
	auto add_text = [scale, &shape](sf::Vector2f pos, sf::Text *text,
		const sf::Color &color = sf::Color::White) {

		text->setScale(scale, -scale);
		text->setPosition(pos);
		text->setColor(color);
		shape.text_elements.push_back(text);
	};

	if(this->get_power_status()){
		shape.elements.emplace_back(item::sprites["power_on"], [this](){this->power_switch(false);});
		shape.elements.emplace_back(item::sprites["active_hotkey_screen"], std::function<void()>());
		shape.elements.emplace_back(item::sprites["active_delay_screen"], std::function<void()>());
		shape.elements.emplace_back(item::sprites["active_button"], std::function<void()>());

		if(this->get_ready(owner)){
			for(uint32_t x = 0; x < 190; x += 38){
				item_button button(item::sprites["progress_bar_ready"], std::function<void()>());
				button.sprite.setPosition(button.sprite.getPosition() + sf::Vector2f(x, 0));
				shape.elements.emplace_back(button);
			}
			add_text(sf::Vector2f(39, 5), &this->name_text);
		} else {
			shape.elements.emplace_back(item::sprites["inactive_button"], std::function<void()>());
			float delay_rate = this->get_delay();
			float progress = 0;
			for(uint32_t x = 0; x < 190; x += 38){
				progress += 0.2;
				if(progress < delay_rate){
					item_button button(item::sprites["progress_bar_reload_ready"], std::function<void()>());
					button.sprite.setPosition(button.sprite.getPosition() + sf::Vector2f(x, 0));
					shape.elements.emplace_back(button);
				} else {
					item_button button(item::sprites["progress_bar_not_ready"], std::function<void()>());
					button.sprite.setPosition(button.sprite.getPosition() + sf::Vector2f(x, 0));
					shape.elements.emplace_back(button);
				}
			}
			add_text(sf::Vector2f(40, 5), &this->name_text, sf::Color(140, 136, 136));
		}
	} else {
		shape.elements.emplace_back(item::sprites["power_off"], [this](){this->power_switch(true);});
		shape.elements.emplace_back(item::sprites["inactive_hotkey_screen"], std::function<void()>());
		shape.elements.emplace_back(item::sprites["inactive_button"], std::function<void()>());
		shape.elements.emplace_back(item::sprites["inactive_delay_screen"], std::function<void()>());

		add_text(sf::Vector2f(40, 5), &this->name_text, sf::Color(140, 136, 136));
	}

	for(auto& button : shape.elements){
		button.sprite.setPosition(button.sprite.getPosition() * scale + position * scale);
	}
	for(auto& text : shape.text_elements){
		text->setPosition(text->getPosition() * scale + position * scale);
	}
	return shape;
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

legs::legs(std::string name)
	: item(name, 1), modes{{0.3f, -2, 1}, {1.0f, -10, 5}, {3.0f, -70, 10}}{

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

		for(auto &sprite : item::sprites){
			sprite.second.setScale(scale, -scale);
		}

		for(auto &sprite : legs::sprites){
			sprite.second.setScale(scale, -scale);
		}

		item_shape shape{};
		if(this->get_power_status()){
			shape.elements.emplace_back(item::sprites["power_on"], [this](){this->power_switch(false);});
			shape.elements.emplace_back(legs::sprites["display_on"], std::function<void()>());
		} else {
			shape.elements.emplace_back(item::sprites["power_off"], [this](){this->power_switch(true);});
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

		for(auto& button : shape.elements){
			button.sprite.setPosition(button.sprite.getPosition() * scale + position * scale);
		}
		return shape;
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
	energy_text("energy_value", get_font(), 22),
	heat_text("heat_value", get_font(), 22){

	std::string path("./../data/");
	std::string filename("mech_60x60x6.png");
	unit::textures[filename].loadFromFile(path + filename);

	this->sprites.emplace_back(
		create_sprite_f(&unit::textures[filename],
		60, 60, 0, 0));

	this->left_arm.emplace_back(std::make_shared<item>("Rocket", 15000));
	this->right_arm.emplace_back(std::make_shared<item>("Rocket", 15000));
	this->torso.emplace_back(std::make_shared<legs>("Legs"));
	this->refresh();
}

void mech::refresh(){
	this->legs_ptr = nullptr;
	for(auto& item : this->left_arm){

	}

	for(auto& item : this->right_arm){

	}

	for(auto& item : this->torso){
		if(item->is_legs()){
			this->legs_ptr = item->is_legs();
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
		sf::Text *text_value, sf::Color value_color){

		add_rectangle(sf::Vector2f(50, 300), offset, sf::Vector2f(0, 0),
			sf::Color(80, 100, 125));
		add_rectangle(sf::Vector2f(50, 300 * (curr_value / max_value)), offset,
			sf::Vector2f(0, 0),	value_color);
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

	add_bar(sf::Vector2f(20, 20), this->status.energy_capacity, this->status.current_energy,
		 &this->energy_text, sf::Color(111, 189, 80));

	add_bar(sf::Vector2f(90, 20), this->status.heat_capacity, this->status.current_heat,
 		 &this->heat_text, sf::Color(136, 52, 14));

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
	get_zone_shape(this->torso, sf::Vector2f{-100, -200});
	get_zone_shape(this->left_arm, sf::Vector2f{-400, -200});
	get_zone_shape(this->right_arm, sf::Vector2f{200, -200});

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

float mech::move_calculate(float time, terrain_en ter_type) noexcept{
	if(!this->legs_ptr->get_power_status())
		return 0;

	float &energy_available = this->status.current_energy;
	float heat_available = this->status.heat_capacity - this->status.current_heat;

	mech_status diff = this->legs_ptr->necessary(time / 10000);
	float energy_necessary = -diff.current_energy;
	const float& heat_necessary = diff.current_heat;

	float rate = 1;
	if((energy_available < energy_necessary) || (heat_available < heat_necessary)){
		float energy_rate = energy_available / energy_necessary;
		float heat_rate = heat_available / heat_necessary;
		rate = energy_rate < heat_rate ? energy_rate : heat_rate;
	}

	this->status.add_current(diff * rate);
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
			item->update(this, time);
		}
	};
	item_update(this->torso);
	item_update(this->left_arm);
	item_update(this->right_arm);
}
