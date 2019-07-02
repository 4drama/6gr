#include "client.hpp"

client::client(game_info *info, int width_, int height_, uint32_t player_index)
	: Width(width_), Height(height_), draw_cells(false), zoom_manager(0),
	window(sf::VideoMode(Width, Height), "game client"), view(){

	view.setSize(Width, -Height);
	view.setCenter(0, 0);
	view.zoom(0.2);

	window.setView(view);
	window.setVerticalSyncEnabled(true);

	view_size = view.getSize();
	display_rate = view_size.x / view_size.y;

	control_players.push_back(player_info(info, player_index));
}

float client::get_view_scale(){
	return this->view_size.x / this->Width;
}

sf::Vector2f client::perspective(sf::Vector2f position){
	float depth = 1000;
	float transform_rate = (depth - position.y) / depth;
	return sf::Vector2f(
		(position.x - view->getCenter().x) * transform_rate + view->getCenter().x,
		(position.y - view->getCenter().y + depth / 3)
		* transform_rate + view->getCenter().y - depth / 3);
}

bool client::on_screen(cell *cell){
	int cull = view->getSize().x;
	if((cell->pos.x > cull) || (cell->pos.x < -cull))
		return false;
	if((cell->pos.y > cull) || (cell->pos.y < -cull))
		return false;
	return true;
}
