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
	effect(game_info *info, std::string filename, uint32_t cell_index, float speed,
		uint32_t columns, uint32_t rows, uint32_t count, sf::Vector2f size);

	void draw(game_info *info, client *client) const noexcept;

	virtual void update(float time) noexcept;
	virtual bool is_life() const noexcept {return true;};
private:
	std::string path;
	uint32_t cell_index;
protected:
	float speed;
	float time_m = 0;
	float anim_time;
};

class one_time_effect : public effect{
public:
	one_time_effect(game_info *info, std::string filename, uint32_t cell_index,
		float speed, uint32_t columns, uint32_t rows, uint32_t count, sf::Vector2f size);

	void update(float time) noexcept override;
	bool is_life() const noexcept  override {return this->time_m < this->anim_time;};
private:
};

class one_frame_effect : public effect{
public:
	one_frame_effect(game_info *info, std::string filename,
		uint32_t cell_index, sf::Vector2f size);

	void update(float time) noexcept override{;};
private:
};

inline std::shared_ptr<effect> create_explosion(game_info *info, uint32_t cell_index);
inline std::shared_ptr<effect> create_crater(game_info *info, uint32_t cell_index);

inline std::shared_ptr<effect> create_explosion(game_info *info, uint32_t cell_index){
	return std::make_shared<one_time_effect>(info, "./../data/explosion.png",
		cell_index, 1.0f, 4, 2, 6, sf::Vector2f(200, 150));
}

inline std::shared_ptr<effect> create_crater(game_info *info, uint32_t cell_index){
	return std::make_shared<one_frame_effect>(info, "./../data/crater.png",
		cell_index, sf::Vector2f(67, 39));
}

#endif
