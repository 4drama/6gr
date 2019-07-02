#include "client.hpp"

player_info::player_info(game_info *info, uint32_t player_) : player(player_){
	this->control_players.emplace_back(player);
	update(info);
}

void player_info::update(game_info *info){
	this->relationship.assign(info->players.size(),
		player_info::relationship_type::NEUTRAL);

	for(auto &player_index : this->control_players){
		this->relationship[player_index] = player_info::relationship_type::CONTROL;
	}

	for(auto &player_index : this->alliance_players){
		this->relationship[player_index] = player_info::relationship_type::ALLIANCE;
	}

	for(auto &player_index : this->enemy_players){
		this->relationship[player_index] = player_info::relationship_type::ENEMY;
	}
}


client::client(game_info *info, int width_, int height_, uint32_t player_index)
	: Width(width_), Height(height_), draw_cells(false), zoom_manager(0),
	window(sf::VideoMode(Width, Height), "game client"),
	view(), player(info, player_index){

	view.setSize(Width, -Height);
	view.setCenter(0, 0);
	view.zoom(0.2);

	window.setView(view);
	window.setVerticalSyncEnabled(true);

	view_size = view.getSize();
	display_rate = view_size.x / view_size.y;
}

float client::get_view_scale(){
	return this->view_size.x / this->Width;
}

sf::Vector2f client::perspective(sf::Vector2f position) const {
	float depth = 1000;
	float transform_rate = (depth - position.y) / depth;
	return sf::Vector2f(
		(position.x - view->getCenter().x) * transform_rate + view->getCenter().x,
		(position.y - view->getCenter().y + depth / 3)
		* transform_rate + view->getCenter().y - depth / 3);
}

bool client::on_screen(cell *cell) const {
	int cull = view->getSize().x;
	if((cell->pos.x > cull) || (cell->pos.x < -cull))
		return false;
	if((cell->pos.y > cull) || (cell->pos.y < -cull))
		return false;
	return true;
}

bool client::is_visable(cell *cell) const {
	if(this->on_screen(cell)){
		for(auto& player_index: this->player.control_players){
			if(cell->player_visible[player_index])
				return true;
		}

		for(auto& player_index: this->player.alliance_players){
			if(cell->player_visible[player_index])
				return true;
		}
	}
	return false;
}

bool client::draw_cell(sf::Vertex *transform_shape, cell *cell,
	std::function<sf::Color(terrain_en)> func) const {

	sf::ConvexShape polygon;
	polygon.setPointCount(6);
	polygon.setFillColor(func(cell.ter.type));

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
		this->perspective(cell->pos));
}

void client::draw_objects(std::vector<sf::Sprite> *object_sprites) const{
	std::sort(object_sprites->begin(), object_sprites->end(),
		[](sf::Sprite f, sf::Sprite s)
		{ return f.getPosition().y > s.getPosition().y;});

	for(auto &sprite : *object_sprites){
		this->window.draw(sprite);
	}
}

void client::draw_cell_border(sf::Vertex *transform_shape) const {
	this->window.draw(transform_shape, 7, sf::LineStrip);
}

std::vector<uint32_t> client::get_vision_indeces(game_info *info) const{
	std::vector<uint32_t> vision_indeces{};

	std::list<uint32_t> vision_players = this->player.control_players;
	{
		std::list<uint32_t> addition_vision_players = this->player.alliance_players;
		vision_players.splice(vision_players.end(), addition_vision_players);
	}

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
	sf::Vector2f view_size = this->view.getSize();
	float scale = this->get_view_scale();
	return sf::Vector2f(pos.x / scale, pos.y / scale);
}
