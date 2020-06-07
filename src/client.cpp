#include "client.hpp"

#include "control.hpp"
#include "gui.hpp"
#include "unit.hpp"

client::client(game_info *info, int width_, int height_, uint32_t player_index)
	: Width(width_), Height(height_), draw_cells(false), zoom_manager(0),
	window(sf::VideoMode(Width, Height), "game client"),
	view(), player(info->players[player_index].info){

	view.setSize(Width, -Height);
	view.setCenter(0, 0);
	view.zoom(0.2);

	window.setView(view);
	window.setVerticalSyncEnabled(true);

	view_size = view.getSize();
	display_rate = view_size.x / view_size.y;

	player_respawn(info, this);
}

float client::get_view_scale() const{
	return this->view_size.x / this->Width;
}

float client::get_view_width() const{
	return this->view_size.x;
}

float client::get_view_height() const{
	return this->view_size.y;
}

int client::get_zoom_state() const{
	return this->zoom_manager;
}

bool client::is_draw_cells() const{
	return this->draw_cells;
}

const player_info& client::get_player_info() const{
	return *player;
}

void client::set_camera(sf::Vector2f pos){
	this->camera_pos = pos;
}

void client::move_camera(cardinal_directions_t dir, float speed){
	using cd_t = cardinal_directions_t;

	sf::Vector2f offset;
	if(dir < cd_t::END)
		offset = cell::offset[int(dir)];
	else if(dir == cd_t::NORTH){
		offset = (cell::offset[(int)cd_t::WEST_NORTH]
			+ cell::offset[(int)cd_t::EAST_NORTH]) / (float)2;
	} else if(dir == cd_t::SOUTH){
		offset = (cell::offset[(int)cd_t::EAST_SOUTH]
			+ cell::offset[(int)cd_t::WEST_SOUTH]) / (float)2;
	}

	offset /= (float)10 / speed;

	this->camera_pos += offset;
}

void client::change_zoom(int value){
	if((value < 0) && (this->zoom_manager > -1)){
		while(value++){
			--this->zoom_manager;
			this->view_size.x += 50;
			this->view_size.y += 50 / this->display_rate;
		}
	} else if((value > 0) && (this->zoom_manager < 3)){
		while(value--){
			++this->zoom_manager;
			this->view_size.x -= 50;
			this->view_size.y -= 50 / this->display_rate;
		}
	}

	this->view.setSize(this->view_size);
}

void client::control_update(game_info *info, float time){
	sf::Event event;
	float speed = 5;

	while(this->window.pollEvent(event)){
		if(event.type == sf::Event::Closed ||
		  (event.type == sf::Event::KeyPressed &&
		   event.key.code == sf::Keyboard::Escape))
			this->window.close();


		static sf::Vector2f last_pasition = sf::Vector2f(0, 0);
		static bool is_map_move = false;
		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Button::Middle){

			is_map_move = true;
			last_pasition = this->mouse_on_map();

		} else if(event.type == sf::Event::MouseMoved ){

			if(is_map_move){
				sf::Vector2f diff = last_pasition - this->mouse_on_map();
				last_pasition = this->mouse_on_map();

				move_camera(cardinal_directions_t::WEST, diff.x * 0.65);
				move_camera(cardinal_directions_t::SOUTH, diff.y * 1.1);
			}
		}
		if (event.type == sf::Event::MouseButtonReleased &&
			event.mouseButton.button == sf::Mouse::Button::Middle){
			is_map_move = false;
		}

		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Left){
			move_camera(cardinal_directions_t::EAST, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Right){
			move_camera(cardinal_directions_t::WEST, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Up){
			move_camera(cardinal_directions_t::SOUTH, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Down){
			move_camera(cardinal_directions_t::NORTH, speed);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Subtract){
			speed_down(info);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Add){
			speed_up(info);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::Space){
			pause(info);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::LBracket){
			this->change_zoom(-1);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::RBracket){
			this->change_zoom(1);
		}
		if(event.type == sf::Event::KeyPressed &&
			event.key.code == sf::Keyboard::F1){
			this->change_show_grid();
		}

		static float left_click_cd = 0;
		left_click_cd -= time;
		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Button::Left){
			if(left_click_cd <= 0){
				if(!gui::instance().gui_interact(info, this)){
					if(!select_item(info, this->player->get_index(), this)){
						select_cell(info, this->player->get_index(), this);
						select_units(info, this->player->get_index());
					}
				}
				left_click_cd = 30;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Button::Right){

			if(info->players[this->player->get_index()].selected_units.size() != 0){
				auto *selected_units = &info->players[this->player->get_index()].selected_units;
				auto finish_cell = get_cell_index_under_mouse(info, this);

				for(auto &curr_unit : *selected_units){
					if((curr_unit.first == this->player->get_index()) && (curr_unit.second.use_count() != 0)){
						std::shared_ptr<unit> unit_ptr = curr_unit.second.lock();

						auto old_front_cell = unit_ptr->path.front();

						unit_ptr->path = path_find(info, unit_ptr->cell_index,
							finish_cell, unit_ptr, this->player->get_index(), true);

						if(old_front_cell != unit_ptr->path.front())
							unit_ptr->path_progress = 0;
					}
				}
			}
		}

		if( event.type == sf::Event::MouseWheelScrolled ){
			this->change_zoom(event.mouseWheelScroll.delta);
		}

	}
}

void client::change_show_grid(){
	this->draw_cells = !this->draw_cells;
}

sf::Vector2f client::perspective(sf::Vector2f position) const {
	float depth = 1000;
	float transform_rate = (depth - position.y) / depth;
	return sf::Vector2f(
		(position.x - this->view.getCenter().x) * transform_rate
		+ this->view.getCenter().x,
		(position.y - this->view.getCenter().y + depth / 3)
		* transform_rate + this->view.getCenter().y - depth / 3);
}

bool client::on_screen(cell *cell) const {
	int cull = this->view.getSize().x;
	if((draw_position(cell, this).x > cull) || (draw_position(cell, this).x < -cull))
		return false;
	if((draw_position(cell, this).y > cull) || (draw_position(cell, this).y < -cull))
		return false;
	return true;
}

bool client::is_visable(cell *cell) const {
	if(this->on_screen(cell)){
		std::list<uint32_t> vision_players = this->player->get_vision_players_indeces();
		for(auto& player_index : vision_players){
			if(cell->player_visible[player_index])
				return true;
		}
	}
	return false;
}

void client::draw(game_info *info, float time){
	if(this->window.isOpen()){
		this->window.clear();

		this->window.setView(this->view);
		draw_map(info, this, time);
		gui::instance().draw(info, this);

		this->show_cursor_point();
		units_draw_paths(info, this);

		const uint32_t player_id = this->player->player;
		const std::vector< player::selected_unit_type > &selected_units =
			info->players[player_id].selected_units;
		if((selected_units.size() == 1) && (selected_units.front().first == player_id)){
			auto unit_ptr = selected_units.front().second.lock();
			unit_ptr->draw_gui(info, this);
		}

		this->window.display();
	}
}

bool client::draw_cell(sf::Vertex *transform_shape, cell *cell,
	std::function<sf::Color(terrain_en)> func) const {

	sf::ConvexShape polygon;
	polygon.setPointCount(6);
	polygon.setFillColor(func(cell->ter.type));

	for(uint32_t i = 0; i < 7; ++i){
		polygon.setPoint(i, transform_shape[i].position);
	}

	if((transform_shape[0].position.y < transform_shape[1].position.y)){
		this->window.draw(polygon);
		if(this->draw_cells)
			this->window.draw(transform_shape, 7, sf::LineStrip);

		return true;
	}
	return false;
}

void client::draw_out_of_view(cell *cell) const	{
	this->window.draw(get_sprite_out_of_view(cell->ter.type,
		this->perspective(draw_position(cell, this))));
}

void client::draw_objects(std::vector<sf::Sprite> *object_sprites) const{
	std::sort(object_sprites->begin(), object_sprites->end(),
		[](sf::Sprite f, sf::Sprite s)
		{ return f.getPosition().y > s.getPosition().y;});

	for(auto &sprite : *object_sprites){
		this->window.draw(sprite);
	}
}

void client::draw_way(sf::Vertex *line, sf::CircleShape *circle, cell *cell) const{
	if(this->on_screen(cell)){
		this->window.draw(line, 2, sf::Lines);
		this->window.draw(*circle);
	}
}

void client::draw_selected_cell(game_info *info) const{
	for(auto &player_index : this->player->control_players){
		auto &curr_player = info->players[player_index];

		for(auto &unit : curr_player.selected_units){
			auto unit_ptr = unit.second.lock();
			if(this->on_screen(&info->map[unit_ptr->cell_index])){

				sf::Vertex transform_shape[7];
				create_transform_shape(this,
					draw_position(&info->map[unit_ptr->cell_index], this),
					transform_shape, sf::Color(88, 244, 122));

				this->draw_cell_border(transform_shape);
			}
		}
	}
}

void client::draw_label(sf::RectangleShape *label) const{
	this->window.draw(*label);
}

void client::draw_cell_border(sf::Vertex *transform_shape) const {
	this->window.draw(transform_shape, 7, sf::LineStrip);
}

void client::draw_buttons(std::vector< button > *buttons) const{
	for(auto &but : *buttons){
		this->window.draw(but.sprites[but.state]);
	}
}

void client::draw_item_shape(const item_shape &shape) const{
	for(auto &sprite : shape.elements){
		this->window.draw(sprite);
	}
}

void client::show_cursor_point(){
	sf::Vector2f pos = this->mouse_on_map();

	sf::CircleShape circle(200);
	circle.setRadius(0.4);
	circle.setOrigin(0.2, 0.2);
	circle.setPosition(pos);
	this->window.draw(circle);
}

std::vector<uint32_t> client::get_vision_indeces(game_info *info) const{
	std::vector<uint32_t> vision_indeces{};

	std::list<uint32_t> vision_players = this->player->get_vision_players_indeces();

	for(auto &curr_player_index : vision_players){
		for(auto &unit : info->players[curr_player_index].units){
			std::copy(unit->vision_indeces.begin(), unit->vision_indeces.end(),
				std::back_inserter(vision_indeces));
		}
	}

	std::sort(vision_indeces.begin(), vision_indeces.end());
	std::unique(vision_indeces.begin(), vision_indeces.end());

	return std::move(vision_indeces);
}

sf::Vector2f client::mouse_on_map() const {
	sf::Vector2i position = sf::Mouse::getPosition() - this->window.getPosition();

	position = sf::Vector2i(position.x, this->Height - position.y)
		- sf::Vector2i(this->Width / 2, this->Height / 2);


	sf::Vector2f pos = sf::Vector2f(position.x - 9, position.y + 29);
	float scale = this->get_view_scale();
	return sf::Vector2f(pos.x * scale, pos.y * scale);
}

sf::Vector2f draw_position(const cell *cell_ptr, const client *client_ptr) noexcept {
	return cell_ptr->pos + client_ptr->get_map_offset();
};
