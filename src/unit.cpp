#include "unit.hpp"

std::map<std::string, sf::Texture> unit::textures{};

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
	: unit(cell_index_, 4){

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

void unit::open_vision(game_info *info, uint32_t player_index){
	this->vision_indeces = open_adjacent(info, player_index,
		this->cell_index, this->vision_range);
}

void unit::unit_update_move(game_info *info, uint32_t player_index, float time){
	if(!this->path.size())
		return;

	terrain_en terr_type = info->get_cell(this->path.front()).ter.type;
	this->path_progress += time * this->speed_mod * this->speed[(int)terr_type];

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
}
