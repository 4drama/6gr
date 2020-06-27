#include "window.hpp"

#include "client.hpp"

#include <iostream>

game_window::game_window(sf::Vector2f position_, sf::Vector2f size_,
	sf::Color color = sf::Color::White)
	: position(position_), size(size_){
	auto add_rectangle = [this](sf::Vector2f size,
		sf::Vector2f offset, sf::Vector2f pos, sf::Color color){

		sf::RectangleShape rect(size);
		rect.setPosition(offset + pos);
		rect.setFillColor(color);
		this->shape.emplace_back(rect);
	};

	add_rectangle(size_, sf::Vector2f(0, 0), position_, color);
}

bool game_window::interact(sf::Vector2f pos, sf::Event event){
	if((event.type == sf::Event::MouseMoved) && is_move){
		this->offset += (pos - old_move_position) / scale;
		this->old_move_position = pos;
		return true;
	}

	if(is_inside(shape.front(), pos) &&
		event.mouseButton.button == sf::Mouse::Button::Left){
		switch(event.type){
			case sf::Event::MouseButtonPressed :
				this->is_move = true;
				this->old_move_position = pos;
			return true;
			case sf::Event::MouseButtonReleased :
				this->is_move = false;
			return true;
		}
		return false;
	} else
		return false;
}

void game_window::draw(sf::RenderWindow *window, float scale){
	this->scale = scale;
	for(sf::RectangleShape &rect : this->shape){
		rect.setSize(this->size * scale);
		rect.setPosition((this->position + this->offset) * scale);
		window->draw(rect);
	}

}
