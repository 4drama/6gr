#include "window.hpp"

#include "client.hpp"

#include <iostream>

sf::Texture game_window::texture{};
std::map<std::string, sf::Sprite> game_window::sprites{};

widget::widget(sf::Vector2f position_, std::map<std::string, sf::Sprite> *sprites_)
	: position(position_), sprites(sprites_){
}


header_bar::header_bar(std::map<std::string, sf::Sprite> *sprites, float length)
	: widget(sf::Vector2f(0, 0), sprites), size(length, 25.0f), main_zone(this->size){
	main_zone.setPosition(this->position);
	main_zone.setFillColor(sf::Color(67, 67, 67));
}

void header_bar::update(game_window *win) noexcept{
	if(win->get_size().x != this->main_zone.getSize().x){
		this->change_length(win->get_size().x);
	}

	this->main_zone.setSize(this->size * win->get_scale());
	this->main_zone.setPosition((this->position + win->get_position()) * win->get_scale());
}

bool header_bar::interact(game_window *win, sf::Vector2f pos, sf::Event event){
	if((event.type == sf::Event::MouseMoved) && is_move){
		win->move((pos - old_move_position) / win->get_scale());
		this->old_move_position = pos;
		return true;
	}

	if(is_inside(main_zone, pos)){
		if(event.mouseButton.button == sf::Mouse::Button::Left){
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
		} else if(event.mouseButton.button == sf::Mouse::Button::Right){
			return true;
		} else
			return false;
	} else
		return false;
}

void header_bar::draw(sf::RenderWindow *window){
	window->draw(main_zone);
}

window::window(std::map<std::string, sf::Sprite> *sprites_, sf::Vector2f size_)
	: widget(sf::Vector2f(0, 0), sprites), size(size_.x, -size_.y), main_zone(this->size){
	main_zone.setPosition(this->position);
	main_zone.setFillColor(sf::Color(167, 167, 167));
}

void window::update(game_window *win) noexcept{
	if(win->get_size() != this->main_zone.getSize()){
		this->change_size(win->get_size());
	}

	this->main_zone.setSize(this->size * win->get_scale());
	this->main_zone.setPosition((this->position + win->get_position()) * win->get_scale());
}

bool window::interact(game_window *win, sf::Vector2f pos, sf::Event event){
	if(is_inside(main_zone, pos)){
		if((event.mouseButton.button == sf::Mouse::Button::Left) ||
			(event.mouseButton.button == sf::Mouse::Button::Right)){
			return true;
		} else
			return false;
	} else
		return false;
}

void window::draw(sf::RenderWindow *window){
	window->draw(main_zone);
}

void game_window::load_sprites(){
	game_window::texture.loadFromFile("./../data/window.png");

	sprites["exit_button"] = sf::Sprite(game_window::texture,
		sf::IntRect(7, 0, 19, 10));
	sprites["exit_button"].setPosition(0, 3);

	/**/
}

game_window::game_window(sf::Vector2f position_, sf::Vector2f size_, sf::Color color)
	: position(position_), size(size_.x, -size_.y){
	if(game_window::sprites.empty())
		game_window::load_sprites();

	widgets.emplace_back(std::make_shared<window>(&game_window::sprites, this->size));
	widgets.emplace_back(std::make_shared<header_bar>(&game_window::sprites, this->size.x));
}

bool game_window::interact(sf::Vector2f pos, sf::Event event){
	for(auto &widget : widgets){
		if(widget->interact(this, pos, event))
			return true;
	}
	return false;
}

void game_window::draw(sf::RenderWindow *window, float scale){
	this->scale = scale;
	for(auto &widget : this->widgets){
		widget->update(this);
		widget->draw(window);
	}

}
