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

content_box::content_box(std::map<std::string, sf::Sprite> *sprites_,
	sf::Vector2f offset_, sf::Vector2f size_)
	: widget(offset_, sprites_), size(size_), main_zone(size_){
	main_zone.setPosition(offset_);
}

void content_box::update(game_window *win) noexcept{
	this->scale = win->get_scale();
	this->offset = win->get_position();

	sf::Vector2f size_ = sf::Vector2f(
		this->size.x == 0 ? win->get_size().x : this->size.x,
		this->size.y == 0 ? win->get_size().y : this->size.y);
	this->main_zone.setSize(size_ * this->scale);
	this->main_zone.setPosition((this->position + win->get_position()) * this->scale);

	for(auto &widget : this->widgets){
		widget->update(this);
	}
}

bool content_box::interact(game_window *win, sf::Vector2f pos, sf::Event event){
	if(is_inside(main_zone, pos)){
		for(auto &widget : this->widgets){
			if(widget->interact(this, pos, event))
				return true;
		}
	}
	return false;
}

void content_box::draw(sf::RenderWindow *window){
	for(auto &widget : this->widgets){
		widget->draw(window);
	}
}

content_box_widget::content_box_widget(sf::Vector2f position_,
	std::map<std::string, sf::Sprite> *sprites_)
	: sprites_ptr(sprites_), position(position_){
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
	if(context_menu_m && context_menu_m->interact(this, pos, event)){
		this->context_menu_m = nullptr;
		return true;
	}

	if(event.type == sf::Event::MouseButtonReleased){
		this->context_menu_m = nullptr;
	}

	for(auto it = this->widgets.rbegin(); it != this->widgets.rend(); ++it){
		if((*it)->interact(this, pos, event)){
			return true;
		}
	}
	return false;
}

void game_window::draw(sf::RenderWindow *window, float scale){
	this->scale = scale;
	for(auto &widget : this->widgets){
		widget->update(this);
		widget->draw(window);
	}

	if(context_menu_m){
		context_menu_m->update(this);
		context_menu_m->draw(window);
	}
}

context_entity::context_entity(sf::Vector2f pos, sf::Color zone_color_,
	std::shared_ptr<sf::Text> text_, std::function<void()> func_)
	: widget(sf::Vector2f(pos), nullptr), text(text_), func(func_), zone_color(zone_color_),
	interact_zone(sf::Vector2f(0, 0)){
	interact_zone.setSize(context_entity::size);
	this->reset_color();
}

void context_entity::update(game_window *win) noexcept{
	auto update_func = [this, win](
		auto &obj, sf::Vector2f offset = sf::Vector2f(0, 0)){

		obj.setScale(win->get_scale(), -win->get_scale());
		obj.setPosition((this->position + win->get_position() + offset)
			* win->get_scale());
	};

	update_func(this->interact_zone);
	update_func(*this->text, sf::Vector2f(3, 3));
}

bool context_entity::interact(game_window *win, sf::Vector2f pos, sf::Event event){
	if(is_inside(this->interact_zone, pos)){
		this->interact_zone.setFillColor(this->zone_color);
		if((event.mouseButton.button == sf::Mouse::Button::Left)
			&& (event.type == sf::Event::MouseButtonReleased)){
			this->func();
			return true;
		}
		return false;
	} else {
		this->reset_color();
		return false;
	}
}

void context_entity::draw(sf::RenderWindow *window){
	if(interact_zone.getFillColor().a != 0){
		window->draw(this->interact_zone);
	}
	if(this->text){
		window->draw(*this->text);
	}
}


context_menu::context_menu(sf::Vector2f position,
	std::map<std::string, sf::Sprite> *sprites,
	deferred_deletion_container<sf::Text> *text_delete_contaier)
	: widget(position, sprites), main_zone(sf::Vector2f(100, 100)),
	chosen_color(sf::Color(207, 207, 207)){
	main_zone.setPosition(position);
	main_zone.setFillColor(sf::Color(107, 107, 107));
	main_zone.setOutlineThickness(1);
	main_zone.setOutlineColor(sf::Color(207, 207, 207));
}

void context_menu::update(game_window *win) noexcept{
	auto update_func = [this, win](
		auto &obj, sf::Vector2f offset = sf::Vector2f(0, 0)){

		obj.setScale(win->get_scale(), -win->get_scale());
		obj.setPosition((this->position + win->get_position() + offset)
			* win->get_scale());
	};

	update_func(this->main_zone);
	for(auto &entity : entities){
		entity.update(win);
	}
}

bool context_menu::interact(game_window *win, sf::Vector2f pos, sf::Event event){
	if(is_inside(this->main_zone, pos)){
		for(auto &entity : entities){
			if(entity.interact(win, pos, event))
				return true;
		}
		return false;
	} else {
		for(auto &entity : entities){
			entity.reset_color();
		}
		return false;
	}
}

void context_menu::draw(sf::RenderWindow *window){
	window->draw(this->main_zone);
	for(auto &entity : entities){
		entity.draw(window);
	}
}

void context_menu::add_entity(
	std::shared_ptr<sf::Text> text, std::function<void()> func_){
	entities.emplace_back(sf::Vector2f(main_zone.getPosition()) +
		sf::Vector2f(0, -context_entity::size.y * this->entities.size()),
		chosen_color, text, func_);
	main_zone.setSize(sf::Vector2f(context_entity::size.x,
		context_entity::size.y * this->entities.size()));
}
