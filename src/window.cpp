#include "window.hpp"

#include "client.hpp"

#include <iostream>

sf::Texture game_window::texture{};
std::map<std::string, sf::Sprite> game_window::sprites{};

widget::widget(sf::Vector2f position_, std::map<std::string, sf::Sprite> *sprites_)
	: position(position_), sprites_ptr(sprites_){
}

exit_button::exit_button(std::map<std::string, sf::Sprite> *sprites)
	: widget(sprites->at("exit_button").getPosition(), sprites)
	, sprite(sprites->at("exit_button")){
}

void exit_button::update(game_window *win) noexcept{
	this->sprite.setScale(sf::Vector2f(1, -1) * win->get_scale());
	this->sprite.setPosition((this->position + win->get_position()
		+ sf::Vector2f(win->get_size().x, 0)) * win->get_scale());
}

bool exit_button::interact(game_window *win, sf::Vector2f pos, sf::Event event){
	if(is_inside(sprite, pos) && event.mouseButton.button == sf::Mouse::Button::Left){
		win->close();
		return true;
	} else
		return false;
}

void exit_button::draw(sf::RenderWindow *window){
	window->draw(sprite);
}

header_bar::header_bar(deferred_deletion_container<sf::Text> *text_delete_contaier,
	std::string title_, std::map<std::string, sf::Sprite> *sprites_, float length)
	: widget(sf::Vector2f(0, 0), sprites_),
	title_ptr(create_text(text_delete_contaier, title_, get_font(), 20)), size(length, 23.0f),
	main_zone(this->size), exit_button_m(sprites_){
	main_zone.setPosition(this->position);
	main_zone.setFillColor(sf::Color(67, 67, 67));

	title_ptr->setFillColor(sf::Color(167, 167, 167));
	title_ptr->setOutlineColor(sf::Color(36, 36, 36));
	title_ptr->setOutlineThickness(1);

	text_delete_contaier->add_pointer(this->title_ptr);
}

void header_bar::update(game_window *win) noexcept{
	tmp_sprites.clear();
	if(win->get_size().x != this->main_zone.getSize().x){
		this->change_length(win->get_size().x);
	}

	this->main_zone.setSize(this->size * win->get_scale());
	this->main_zone.setPosition((this->position + win->get_position()) * win->get_scale());

	title_ptr->setScale(win->get_scale(), -win->get_scale());
	title_ptr->setPosition((this->position + sf::Vector2f{3, 25} + win->get_position())
		* win->get_scale());

	auto create_sprite = [&win](const sf::Sprite &sprite,
		sf::Vector2f scale = sf::Vector2f(1.0f, -1.0f),
		sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f)){
		sf::Sprite res_sprite = sprite;
		res_sprite.setScale(scale * win->get_scale());
		res_sprite.setPosition((sprite.getPosition() +
			win->get_position() + offset) * win->get_scale());
		return res_sprite;
	};

	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_up_side"),
		sf::Vector2f(this->size.x - 6, -1)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_left_up_corner")));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_right_up_corner"),
		sf::Vector2f(1.0f, -1.0f), sf::Vector2f(this->size.x, 0.0f)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_left_side"),
		sf::Vector2f(1, -(this->size.y -3))));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_right_side"),
		sf::Vector2f(1, -(this->size.y -3)), sf::Vector2f(this->size.x, 0.0f)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_left_down_corner")));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_right_down_corner"),
		sf::Vector2f(1.0f, -1.0f), sf::Vector2f(this->size.x, 0.0f)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("header_bar_down_side"),
		sf::Vector2f(this->size.x - 6, -1)));

	this->exit_button_m.update(win);
}

bool header_bar::interact(game_window *win, sf::Vector2f pos, sf::Event event){
	if((event.type == sf::Event::MouseMoved) && is_move){
		win->move((pos - old_move_position) / win->get_scale());
		this->old_move_position = pos;
		return true;
	}

	if(is_inside(main_zone, pos)){
		if(exit_button_m.interact(win, pos, event))
			return true;
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
	for(auto &sprite : this->tmp_sprites){
		window->draw(sprite);
	}
	this->exit_button_m.draw(window);
	window->draw(*title_ptr);
}

window::window(std::map<std::string, sf::Sprite> *sprites_, sf::Vector2f size_)
	: widget(sf::Vector2f(0, 0), sprites_), size(size_.x, -size_.y), main_zone(this->size){
	main_zone.setPosition(this->position);
	main_zone.setFillColor(sf::Color(167, 167, 167));
}

void window::update(game_window *win) noexcept{
	tmp_sprites.clear();
	if(win->get_size() != this->main_zone.getSize()){
		this->change_size(win->get_size());
	}

	this->main_zone.setSize(this->size * win->get_scale());
	this->main_zone.setPosition((this->position + win->get_position()) * win->get_scale());

	auto create_sprite = [&win](const sf::Sprite &sprite,
		sf::Vector2f scale = sf::Vector2f(1.0f, -1.0f),
		sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f)){
		sf::Sprite res_sprite = sprite;
		res_sprite.setScale(scale * win->get_scale());
		res_sprite.setPosition((sprite.getPosition() +
			win->get_position() + offset) * win->get_scale());
		return res_sprite;
	};

	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("window_left_side"),
		sf::Vector2f(1, this->size.y + 3)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("window_right_side"),
		sf::Vector2f(1, this->size.y + 3), sf::Vector2f(this->size.x, 0.0f)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("window_down_side"),
		sf::Vector2f(this->size.x - 6, -1), sf::Vector2f(0, this->size.y)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("window_left_down_corner"),
		sf::Vector2f(1.0f, -1.0f), sf::Vector2f(0, this->size.y)));
	tmp_sprites.emplace_back(create_sprite(sprites_ptr->at("window_right_down_corner"),
		sf::Vector2f(1.0f, -1.0f), sf::Vector2f(this->size.x, this->size.y)));
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

	for(auto &sprite : this->tmp_sprites){
		window->draw(sprite);
	}
}

void game_window::load_sprites(){
	game_window::texture.loadFromFile("./../data/window.png");

	sprites["exit_button"] = sf::Sprite(game_window::texture,
		sf::IntRect(7, 0, 19, 19));
	sprites["exit_button"].setPosition(-22, 22);

	sprites["header_bar_up_side"] = sf::Sprite(game_window::texture,
		sf::IntRect(3, 0, 1, 3));
	sprites["header_bar_up_side"].setPosition(3, 25);

	sprites["header_bar_left_up_corner"] = sf::Sprite(game_window::texture,
		sf::IntRect(0, 0, 3, 3));
	sprites["header_bar_left_up_corner"].setPosition(0, 25);

	sprites["header_bar_right_up_corner"] = sf::Sprite(game_window::texture,
		sf::IntRect(4, 0, 3, 3));
	sprites["header_bar_right_up_corner"].setPosition(-3, 25);

	sprites["header_bar_left_side"] = sf::Sprite(game_window::texture,
		sf::IntRect(0, 3, 3, 1));
	sprites["header_bar_left_side"].setPosition(0, 22);

	sprites["header_bar_right_side"] = sf::Sprite(game_window::texture,
		sf::IntRect(4, 3, 3, 1));
	sprites["header_bar_right_side"].setPosition(-3, 22);

	sprites["header_bar_left_down_corner"] = sf::Sprite(game_window::texture,
		sf::IntRect(0, 22, 3, 3));
	sprites["header_bar_left_down_corner"].setPosition(0, 3);

	sprites["header_bar_right_down_corner"] = sf::Sprite(game_window::texture,
		sf::IntRect(4, 22, 3, 3));
	sprites["header_bar_right_down_corner"].setPosition(-3, 3);

	sprites["header_bar_down_side"] = sf::Sprite(game_window::texture,
		sf::IntRect(3, 22, 1, 3));
	sprites["header_bar_down_side"].setPosition(3, 3);

	sprites["window_left_side"] = sf::Sprite(game_window::texture,
		sf::IntRect(0, 25, 3, 1));
	sprites["window_left_side"].setPosition(0, 0);

	sprites["window_right_side"] = sf::Sprite(game_window::texture,
		sf::IntRect(4, 25, 3, 1));
	sprites["window_right_side"].setPosition(-3, 0);

	sprites["window_down_side"] = sf::Sprite(game_window::texture,
		sf::IntRect(3, 26, 1, 3));
	sprites["window_down_side"].setPosition(3, 3);

	sprites["window_left_down_corner"] = sf::Sprite(game_window::texture,
		sf::IntRect(0, 26, 3, 3));
	sprites["window_left_down_corner"].setPosition(0, 3);

	sprites["window_right_down_corner"] = sf::Sprite(game_window::texture,
		sf::IntRect(4, 26, 3, 3));
	sprites["window_right_down_corner"].setPosition(-3, 3);
}

game_window::game_window(deferred_deletion_container<sf::Text> *text_delete_contaier,
	std::string title, sf::Vector2f position_,
	sf::Vector2f size_, sf::Color color)
	: position(position_), size(size_.x, -size_.y){
	if(game_window::sprites.empty())
		game_window::load_sprites();

	widgets.emplace_back(std::make_shared<window>(&game_window::sprites, this->size));
	widgets.emplace_back(std::make_shared<header_bar>
		(text_delete_contaier, title, &game_window::sprites, this->size.x));
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

std::shared_ptr<sf::Text> create_text(
	deferred_deletion_container<sf::Text> *text_delete_contaier,
	const sf::String &string, const sf::Font &font, unsigned int characterSize){
	std::shared_ptr<sf::Text> res =
		std::make_shared<sf::Text>(string, font, characterSize);
	text_delete_contaier->add_pointer(res);
	return res;
}
