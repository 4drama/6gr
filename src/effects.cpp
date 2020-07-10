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

effect::effect(uint32_t cell_index_)
	: cell_index(cell_index_), speed(1.0f / 1000.0f){
	this->anim = std::make_shared<animation>("./../data/explosion.png",
		4, 2, 6, sf::Vector2f(200, 150));
}

void effect::update(float time) noexcept {
	this->lifetime += time * this->speed;
};

void effect::draw(game_info *info, client *client) const noexcept{
	const cell& cell = info->get_cell(this->cell_index);
	if(client->is_visable(this->cell_index) && client->on_screen(&cell)){
		sf::Sprite sprite = anim->get_frame(std::floor(lifetime));
		client->set_camera_offset(sprite, cell.pos);
		client->prepare_to_draw(sprite);
		client->get_window().draw(sprite);
	}
}
