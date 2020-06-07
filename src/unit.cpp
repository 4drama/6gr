#include "unit.hpp"

#include <iostream>

sf::Texture item::texture{};
std::map<std::string, sf::Sprite> item::sprites;

std::map<std::string, sf::Texture> unit::textures{};

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
		sf::IntRect(197, 23, 15, 35));
	sprites["power_off"].setPosition(0, 0);

	sprites["power_on"] = sf::Sprite(item::texture,
		sf::IntRect(212, 23, 15, 35));
	sprites["power_on"].setPosition(0, 0);

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
	: name(name_), delay(delay_){
	if(item::sprites.empty())
		item::load_sprites();
}

bool item::get_ready() const noexcept{
	bool status = false;
	if(this->get_power_status() && (this->get_delay() >= 1) && this->has_ammo()){
		status = true;
	}
	return status;
}

void item::update(float time){
	if(this->curr_delay < this->delay){
		this->curr_delay += time;
	}
}

item_shape item::get_draw_shape(client *client, const sf::Vector2f& position) const{
	float scale = client->get_view_scale();

	for(auto &sprite : item::sprites){
		sprite.second.setScale (scale, -scale);
	}

	item_shape shape{};
	if(this->get_power_status()){
		shape.elements.push_back(item::sprites["power_on"]);
		shape.elements.push_back(item::sprites["active_hotkey_screen"]);
		shape.elements.push_back(item::sprites["active_delay_screen"]);
		if(this->get_ready()){
			shape.elements.push_back(item::sprites["active_button"]);
			for(uint32_t x = 0; x < 190; x += 38){
				sf::Sprite sprite(item::sprites["progress_bar_ready"]);
				sprite.setPosition(sprite.getPosition() + sf::Vector2f(x, 0));
				shape.elements.push_back(sprite);
			}
		} else {
			shape.elements.push_back(item::sprites["inactive_button"]);
			float delay_rate = this->get_delay();
			float progress = 0;
			for(uint32_t x = 0; x < 190; x += 38){
				progress += 0.2;
				if(progress < delay_rate){
					sf::Sprite sprite(item::sprites["progress_bar_reload_ready"]);
					sprite.setPosition(sprite.getPosition() + sf::Vector2f(x, 0));
					shape.elements.push_back(sprite);
				} else {
					sf::Sprite sprite(item::sprites["progress_bar_not_ready"]);
					sprite.setPosition(sprite.getPosition() + sf::Vector2f(x, 0));
					shape.elements.push_back(sprite);
				}
			}
		}
	} else {
		shape.elements.push_back(item::sprites["power_off"]);
		shape.elements.push_back(item::sprites["inactive_hotkey_screen"]);
		shape.elements.push_back(item::sprites["inactive_button"]);
		shape.elements.push_back(item::sprites["inactive_delay_screen"]);
	}

	for(auto& sprite : shape.elements){
		sprite.setPosition(sprite.getPosition() * scale + position * scale);
	}
	return shape;
}

/*void item::draw_button(game_info *info, client *client,
	sf::Vector2f position) const noexcept{


}

void item::push_button(game_info *info, client *client,
	sf::Vector2f but_position, sf::Vector2f click_position) const noexcept{

}*/

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

/*std::shared_ptr<unit> unit::create_caravan(unit::weight_level_type weight,
	uint32_t cell_index_){

	std::string path("./../data/");
	std::string filename("caravan_60x60x8.png");
	unit::textures[filename].loadFromFile(path + filename);

	unit result{};
	result.vision_range = 3;
	result.cell_index = cell_index_;
	result.sprites.emplace_back(
		create_sprite_f(&unit::textures[filename],
		60, 60, 0, 0));



	switch(weight){
		case unit::weight_level_type::LIGHT :
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[filename],
				60, 60, 2, 0));
			result.speed_mod = (float)2 / 10000;
		break;
		case unit::weight_level_type::LOADED :
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[filename],
				60, 60, 1, 0));
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[filename],
				60, 60, 0, 1));
			result.speed_mod = (float)1 / 10000;
		break;
		case unit::weight_level_type::OVERLOADED :
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[filename],
				60, 60, 1, 0));
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[filename],
				60, 60, 0, 1));
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[filename],
				60, 60, 1, 1));
			result.speed_mod = (float)0.5 / 10000;
		break;
	}

	result.speed[(int)terrain_en::RIVER] = 0;
	result.speed[(int)terrain_en::MOUNTAIN] = 0.5;
	result.speed[(int)terrain_en::PLAIN] = 2;
	result.speed[(int)terrain_en::PALM] = 2;

	return std::make_shared<unit>(result);
}*/

unit::unit(uint32_t cell_index_, uint32_t vision_range_)
	: cell_index(cell_index_), vision_range(vision_range_){
}

mech::mech(uint32_t cell_index_)
	: unit(cell_index_, 4), weapon("Rocket", 15000){

	std::string path("./../data/");
	std::string filename("mech_60x60x6.png");
	unit::textures[filename].loadFromFile(path + filename);

	this->sprites.emplace_back(
		create_sprite_f(&unit::textures[filename],
		60, 60, 0, 0));

	this->speed[(int)terrain_en::RIVER] = 0;
	this->speed[(int)terrain_en::MOUNTAIN] = 0.5;
	this->speed[(int)terrain_en::PLAIN] = 2;
	this->speed[(int)terrain_en::PALM] = 2;
}

void mech::draw_gui(game_info *info, client *client){
	float scale = client->get_view_scale();
	auto shape  = weapon.get_draw_shape(client, sf::Vector2f{-100, -200});
	client->draw_item_shape(shape);
}

bool mech::interact_gui(game_info *info, client *client){
	float scale = client->get_view_scale();
	sf::Vector2f pos = client->mouse_on_map();
	auto shape = this->weapon.get_draw_shape(client, sf::Vector2f{-100, -200});
	for(auto &sprite : shape.elements){
		if(is_inside_sprite(sprite, pos)){
			std::cerr << "click" << std::endl;
			return true;
		}
	}
	return false;
}

void unit::open_vision(game_info *info, uint32_t player_index){
	this->vision_indeces = open_adjacent(info, player_index,
		this->cell_index, this->vision_range);
}

void unit::unit_update_move(game_info *info, uint32_t player_index, float time){
	if(!this->path.size())
		return;

	terrain_en terr_type = info->get_cell(this->path.front()).ter.type;
	this->path_progress += time * this->get_speed(terr_type);

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
	weapon.update(time);
}
