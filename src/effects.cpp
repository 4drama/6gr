#include "effects.hpp"

#include <cmath>

#include <iostream>

animation::animation(std::string path, uint32_t columns, uint32_t rows,
	uint32_t count, sf::Vector2f size){
	this->texture.loadFromFile(path);

	uint32_t counter = 0;
	for(uint32_t row = 0; row < rows; ++row){
		for(uint32_t col = 0; col < columns; ++col){
			this->sprites.emplace_back(this->texture,
				sf::IntRect(col * size.x, row * size.y, size.x, size.y));
			if(++counter == count){
				break;
			}
		}
	}

	for(auto &sprite : this->sprites){
		sprite.setOrigin(size.x / 2, size.y / 2);
		sprite.setScale(0.2, -0.2);
	}
}

effect::effect(game_info *info, std::string path_, uint32_t cell_index_, float speed_,
	uint32_t columns, uint32_t rows, uint32_t count, sf::Vector2f size)
	: path(path_), speed(speed_ / 1000.0f), cell_index(cell_index_), anim_time(count){

	if(!info->animations[path_]){
		info->animations[path_] =
			std::make_shared<animation>(path_, columns, rows, count, size);
	}
}

void effect::draw(game_info *info, client *client) const noexcept{
	const cell& cell = info->get_cell(this->cell_index);
	if(client->is_visable(this->cell_index) && client->on_screen(&cell)){
		sf::Sprite sprite = info->animations[path]->get_frame(std::floor(time_m));
		client->set_camera_offset(sprite, cell.pos);
		client->prepare_to_draw(sprite);
		client->get_window().draw(sprite);
	}
}

void effect::update(float time) noexcept{
	this->time_m += time * this->speed;
	if(this->time_m > this->anim_time){
		this->time_m -= this->anim_time;
	}
}

one_time_effect::one_time_effect(game_info *info, std::string filename,
	uint32_t cell_index, float speed, uint32_t columns, uint32_t rows,
	uint32_t count, sf::Vector2f size)
	: effect(info, filename, cell_index, speed, columns, rows, count, size) {
}

void one_time_effect::update(float time) noexcept{
	this->time_m += time * this->speed;
}

one_frame_effect::one_frame_effect(game_info *info, std::string filename,
	 uint32_t cell_index, sf::Vector2f size)
	 : effect(info, filename, cell_index, 0, 1, 1, 1, size){
}
