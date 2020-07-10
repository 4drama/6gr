#ifndef EFFECTS_HPP
#define EFFECTS_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

#include <memory>
#include <vector>
#include <string>

class animation{
public:
	animation(std::string path, uint32_t columns,
		uint32_t rows, uint32_t count, sf::Vector2f size);

	sf::Sprite get_frame(uint32_t i) const {return sprites.at(i);};
	uint32_t get_size() const noexcept {return sprites.size();};
private:
	sf::Texture texture;
	std::vector<sf::Sprite> sprites;
};

class effect{
public:
	effect(uint32_t cell_index_);

	void draw(game_info *info, client *client) const noexcept;
	void update(float time) noexcept;
	bool is_life() const noexcept {return this->lifetime < this->max_lifetime;};
private:
	float speed;
	std::shared_ptr<animation> anim;

	float lifetime = 0;
	float max_lifetime = 6;

	uint32_t cell_index;
};

#endif
