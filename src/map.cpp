#include "map.hpp"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <stdexcept>

const float pi = 3.14159265359;

float cell::side_size = 0;
sf::Vector2f cell::offset[(int)cardinal_directions_t::END] = {};
sf::Vertex cell::shape[7] = {};
uint32_t terrain::gen_rate[(int)terrain_en::END][(int)terrain_en::END] = {};
std::vector<sf::Texture> object::textures{};
std::vector<sf::Texture> unit::textures{};

cardinal_directions_t against(cardinal_directions_t dir){
	using cd_t = cardinal_directions_t;
	cardinal_directions_t result = ((int)dir + 3) < (int)cd_t::END
		? (cd_t)((int)dir + 3) : (cd_t)((int)dir - 3);
	return result;
}

cardinal_directions_t next(cardinal_directions_t dir){
	using cd_t = cardinal_directions_t;
	return dir != cd_t::LAST ? (cd_t)((int)dir + 1) : cd_t::BEGIN;
}

cardinal_directions_t previous(cardinal_directions_t dir){
	using cd_t = cardinal_directions_t;
	return dir != cd_t::BEGIN ? (cd_t)((int)dir - 1) : cd_t::LAST;
}

struct connection{
	cardinal_directions_t dir;
	uint32_t index;
};

void generate_terrain(std::vector<cell> *map, uint32_t index);
uint32_t create_cell(std::vector<cell> *map, std::vector<connection> *conn);
void add_connection(cell *cell, connection con);

void print_info(std::vector<cell> *map, uint32_t index){
	if(index != UINT32_MAX){
		cell *curr_cell = &map->at(index);
		std::cerr << index << " " << (int)curr_cell->ter.type
			<< " x = " << curr_cell->pos.x << ". y = " << curr_cell->pos.y
			<< std::endl;
	}
}

void generate_level(std::vector<cell> *map){
	using cd_t = cardinal_directions_t;
	terrain::fill_gen_rate();

	uint32_t start_index = map->size() - 1;
	uint32_t current_index = start_index;

	uint32_t last_add_index = UINT32_MAX;

	cd_t dir = cd_t::BEGIN;
	bool circle_flag = false;
	while(true){
		generate_terrain(map, last_add_index);
		if(map->at(current_index).indeces[(int)dir] == UINT32_MAX){

			std::vector<connection> conn{};
			conn.emplace_back(connection{next(against(dir)), last_add_index});
			conn.emplace_back(connection{against(dir), current_index});

			uint32_t index = create_cell(map, &conn);

			add_connection(&map->at(current_index),
				connection{dir, index});

			if(last_add_index != UINT32_MAX){
				add_connection(&map->at(last_add_index),
					connection{next(dir), index});
			}

			uint32_t next_chain_index = map->at(current_index).indeces[(int)next(dir)];
			last_add_index = index;
			if( (next_chain_index != UINT32_MAX)
				&& (next_chain_index != (start_index + 1)) ){

				add_connection(&map->at(next_chain_index),
					connection{previous(dir), index});
				add_connection(&map->at(index),
					connection{against(previous(dir)), next_chain_index});

				current_index = next_chain_index;
				continue;
			}

		}

		if( (next(dir) == cd_t::BEGIN) && (circle_flag)){
			uint32_t start_chain_index = start_index + 1;

			add_connection(&map->at(last_add_index),
				connection{against(previous(dir)), start_chain_index});

			add_connection(&map->at(start_chain_index),
				connection{previous(dir), last_add_index});
			break;
		}

		dir = next(dir);
		if(dir == cd_t::LAST){
			circle_flag = true;
		}
	}
	generate_terrain(map, last_add_index);
}

void terrain::fill_gen_rate(){
	terrain::gen_rate[(int)terrain_en::RIVER][(int)terrain_en::RIVER] = 100;
	terrain::gen_rate[(int)terrain_en::RIVER][(int)terrain_en::MOUNTAIN] = 0;
	terrain::gen_rate[(int)terrain_en::RIVER][(int)terrain_en::PLAIN] = 20;
	terrain::gen_rate[(int)terrain_en::RIVER][(int)terrain_en::PALM] = 200;

	terrain::gen_rate[(int)terrain_en::MOUNTAIN][(int)terrain_en::RIVER] = 0;
	terrain::gen_rate[(int)terrain_en::MOUNTAIN][(int)terrain_en::MOUNTAIN] = 90;
	terrain::gen_rate[(int)terrain_en::MOUNTAIN][(int)terrain_en::PLAIN] = 40;
	terrain::gen_rate[(int)terrain_en::MOUNTAIN][(int)terrain_en::PALM] = 0;

	terrain::gen_rate[(int)terrain_en::PLAIN][(int)terrain_en::RIVER] = 1;
	terrain::gen_rate[(int)terrain_en::PLAIN][(int)terrain_en::MOUNTAIN] = 30;
	terrain::gen_rate[(int)terrain_en::PLAIN][(int)terrain_en::PLAIN] = 170;
	terrain::gen_rate[(int)terrain_en::PLAIN][(int)terrain_en::PALM] = 0;

	terrain::gen_rate[(int)terrain_en::PALM][(int)terrain_en::RIVER] = 0;
	terrain::gen_rate[(int)terrain_en::PALM][(int)terrain_en::MOUNTAIN] = 10;
	terrain::gen_rate[(int)terrain_en::PALM][(int)terrain_en::PLAIN] = 100;
	terrain::gen_rate[(int)terrain_en::PALM][(int)terrain_en::PALM] = 0;

	for(terrain_en terr = terrain_en::BEGIN; terr < terrain_en::END;
		terr = (terrain_en)((int)terr + 1)){

		uint32_t amount = 0;
		for(terrain_en gen = terrain_en::BEGIN;
			gen < terrain_en::END; gen = (terrain_en)((int)gen + 1)){

			amount += terrain::gen_rate[(int)terr][(int)gen];
		}

		for(terrain_en gen = terrain_en::BEGIN;
			gen < terrain_en::END; gen = (terrain_en)((int)gen + 1)){

			terrain::gen_rate[(int)terr][(int)gen] /= (float)amount / 100;
		}
	}
}

namespace{

void add_gen_rate_f(uint32_t *amount_gen_rate, terrain_en type){
	for(terrain_en add = terrain_en::BEGIN;
		add < terrain_en::END; add = (terrain_en)((int)add + 1)){

		amount_gen_rate[(int)add] += terrain::gen_rate[(int)type][(int)add];
	}
}

terrain_en gen_ter_f(std::vector<cell> *map, uint32_t index){
	using cd_t = cardinal_directions_t;

	uint32_t amount_gen_rate[(int)terrain_en::END] = {};
	for(cd_t dir = cd_t::BEGIN; dir != cd_t::END ; dir = (cd_t)((int)dir + 1)){
		uint32_t dir_index = map->at(index).indeces[(int)dir];
		if(dir_index != UINT32_MAX){
			add_gen_rate_f(amount_gen_rate, map->at(dir_index).ter.type);
		}
	}

	uint32_t weight = 0;
	for(terrain_en add = terrain_en::BEGIN;
		add < terrain_en::END; add = (terrain_en)((int)add + 1)){

		weight += amount_gen_rate[(int)add];
	}

	int hit = rand() % weight;
	for(terrain_en curr_ter = terrain_en::BEGIN; curr_ter < terrain_en::END;
		curr_ter = (terrain_en)((int)curr_ter + 1)){

		hit -= amount_gen_rate[(int)curr_ter];
		if(hit <= 0){
			return curr_ter;
		}
	}
}

float rand_float_f(float max){
	return (float)rand()/(float)(RAND_MAX / max);
}

sf::Vector2f get_random_pos_f(float size){
	float border = abs(cell::shape[0].position.x) - (size / 2);
	float distance = rand_float_f(border);
	float angle = rand_float_f(pi * 2);
	sf::Vector2f result{std::cos(angle) * distance, std::sin(angle) * distance};
	return result;
}

float get_distance_f(sf::Vector2f fp, sf::Vector2f sp){
	sf::Vector2f side = fp - sp;
	return sqrt(pow(side.x, 2) + pow(side.y, 2));
}

sf::Vector2f get_free_pos_f(std::vector<object> *objects, float size){
	while(true){
		sf::Vector2f pos = get_random_pos_f(size);
		bool is_blocked = false;
		for(auto &obj : *objects){
			if( get_distance_f(pos, obj.pos) < (size + obj.size) ){
				is_blocked = true;
				break;
			}
		}
		if(!is_blocked)
			return pos;
	}
}

void create_obect_witch_chance_f(
	std::function< object( std::vector<object>* ) > fun,
	std::vector<object> *objects,
	float chance){


	float hit = rand_float_f(1);
	if(hit <= chance)
		objects->emplace_back(fun(objects));
}

void generate_objects_f(terrain *ter){
	switch(ter->type){
	case terrain_en::PALM:
		ter->objects.clear();
		create_obect_witch_chance_f(&object::create_palm, &ter->objects, 1);
		create_obect_witch_chance_f(&object::create_palm, &ter->objects, 0.3);
		create_obect_witch_chance_f(&object::create_palm, &ter->objects, 0.3);
		create_obect_witch_chance_f(&object::create_palm, &ter->objects, 0.3);
		break;
	case terrain_en::MOUNTAIN:
		ter->objects.clear();
		create_obect_witch_chance_f(&object::create_mountain, &ter->objects, 1);
		break;
	default:
		ter->objects.clear();
	}
}

void regenerate_terrain_f(std::vector<cell> *map, uint32_t index){
	using cd_t = cardinal_directions_t;

	cell *current_cell = &map->at(index);
	if(current_cell->ter.type != terrain_en::RIVER)
		return;
	for(cd_t dir = cd_t::BEGIN; dir != cd_t::END ; dir = (cd_t)((int)dir + 1)){
		uint32_t dir_index = current_cell->indeces[(int)dir];
		if((dir_index != UINT32_MAX)
			&& map->at(dir_index).ter.type != terrain_en::RIVER){
			map->at(dir_index).ter.type = gen_ter_f(map, dir_index);
			generate_objects_f(&map->at(dir_index).ter);
		}
	}
}

}

namespace{

sf::Sprite create_palm_sprite_f(int step){

	sf::Sprite frame;
	int width = 50;
	int height = 70;
	sf::IntRect rectangle{0 + width * step, 0,
		 width, height};

	frame.setTexture(object::textures[(int)object::texture_type::PALM]);
	frame.setTextureRect(rectangle);
	frame.setOrigin(25, 69);
	frame.setScale (0.2, -0.2);

	return frame;
}

sf::Sprite create_mountain_sprite_f(int step){

	sf::Sprite frame;
	int width = 50;
	int height = 50;
	int selected_sprite_index = rand() % 2;

	sf::IntRect rectangle{0 + width * selected_sprite_index, 0,
		 width, height};

	frame.setTexture(object::textures[(int)object::texture_type::MOUNTAIN]);
	frame.setTextureRect(rectangle);
	frame.setOrigin(25, 45);
	frame.setScale(0.35, -0.35);
	return frame;
}

}

object object::create_palm(std::vector<object> *objects){
	object result{};
	result.size = 1.5;
	result.pos = get_free_pos_f(objects, result.size);

	int frames_count = 2;
	float time_step = 2500;
	for(int curr_step = 0; curr_step < frames_count; ++curr_step){

		 result.sprites.emplace_back(
 			std::make_pair(time_step + curr_step * time_step,
			create_palm_sprite_f(curr_step)));
	}

	result.last_time_point = time_step * frames_count;
	result.time_point = rand_float_f(result.last_time_point / 4);
	return result;
}

object object::create_mountain(std::vector<object> *objects){
	object result{};
	result.size = 2.5;
	result.pos = sf::Vector2f{0, -3};

	int frames_count = 1;
	float time_step = 2500;
	for(int curr_step = 0; curr_step < frames_count; ++curr_step){

		 result.sprites.emplace_back(
 			std::make_pair(time_step + curr_step * time_step,
			create_mountain_sprite_f(curr_step)));
	}

	result.last_time_point = time_step * frames_count;
	result.time_point = rand_float_f(result.last_time_point / 4);

	return result;
}

sf::Sprite object::update_sprite(float time){
	this->time_point += time;
	while(this->time_point >= this->last_time_point){
		this->time_point -= this->last_time_point;
	}

	for(auto point_and_sprite : this->sprites){
		if(this->time_point < point_and_sprite.first){
			return point_and_sprite.second;
		}
	}

	throw std::runtime_error("sprite could not fount.");
}

void generate_terrain(std::vector<cell> *map, uint32_t index){
	using cd_t = cardinal_directions_t;

	if(index == UINT32_MAX)
		return;

	map->at(index).ter.type = gen_ter_f(map, index);
	generate_objects_f(&map->at(index).ter);
	regenerate_terrain_f(map, index);
}

uint32_t create_cell(std::vector<cell> *map, std::vector<connection> *conn){
	cell new_cell{};

	for(auto& connection : *conn){
		add_connection(&new_cell, connection);
	}

	uint32_t index = map->size();
	new_cell.pos = map->at(conn->at(1).index).pos
		+ cell::offset[(int)against(conn->at(1).dir)];

	map->emplace_back(new_cell);
	return index;
}

void add_connection(cell *cell, connection con){
	cell->indeces[(int)con.dir] = (int)con.index;
}

void cell::set_side_size(float side_size_){
	using cd_t = cardinal_directions_t;

	cell::side_size = side_size_;

	const float horis_dist = cell::side_size * std::sin(pi / 180 * 60) * 2;
	const float vert_dist
		= cell::side_size * std::cos(pi / 180 * 60) + cell::side_size;

	cell::offset[(int)cd_t::WEST] = sf::Vector2f{-horis_dist, 0};
	cell::offset[(int)cd_t::WEST_NORTH] = sf::Vector2f{-horis_dist / 2, vert_dist};
	cell::offset[(int)cd_t::EAST_NORTH] = sf::Vector2f{horis_dist / 2, vert_dist};
	cell::offset[(int)cd_t::EAST] = sf::Vector2f{horis_dist, 0};
	cell::offset[(int)cd_t::EAST_SOUTH] = sf::Vector2f{horis_dist / 2, -vert_dist};
	cell::offset[(int)cd_t::WEST_SOUTH] = sf::Vector2f{-horis_dist / 2, -vert_dist};

	const float short_rate = std::cos(pi / 180 * 60);
	const float long_rate = std::sin(pi / 180 * 60);

	cell::shape[0] = sf::Vertex(sf::Vector2f( -side_size * long_rate, -side_size / 2 ) );
	cell::shape[1] = sf::Vertex(sf::Vector2f( -side_size * long_rate, side_size / 2 ) );
	cell::shape[2] = sf::Vertex(sf::Vector2f( 0, (side_size / 2) + (side_size * short_rate) ) );
	cell::shape[3] = sf::Vertex(sf::Vector2f( side_size * long_rate, side_size / 2 ) );
	cell::shape[4] = sf::Vertex(sf::Vector2f( side_size * long_rate, -side_size / 2 ) );
	cell::shape[5] = sf::Vertex(sf::Vector2f( 0, (-side_size / 2) + (-side_size * short_rate)) );
	cell::shape[6] = sf::Vertex(sf::Vector2f( -side_size * long_rate, -side_size / 2 ) );
}

void draw_path(sf::RenderWindow *window, cell *curr_cell, std::vector<cell> *map){
	using cd_t = cardinal_directions_t;
	for(cd_t dir = cd_t::BEGIN; dir < cd_t::EAST ; dir = next(dir)){
		if(curr_cell->indeces[(int)dir] != UINT32_MAX){
			sf::Vertex line[] = {
			    sf::Vertex(sf::Vector2f(2, 2) + curr_cell->pos),
			    sf::Vertex(sf::Vector2f(2, -2) + map->at(curr_cell->indeces[(int)dir]).pos) };

			line[0].color = sf::Color(100, 250, 50);
			line[1].color = sf::Color(100, 250, 50);

			window->draw(line, 2, sf::Lines);
		}
	}

	for(cd_t dir = cd_t::EAST; dir != cd_t::END ; dir = (cd_t)((int)dir + 1)){
		if(curr_cell->indeces[(int)dir] != UINT32_MAX){
			sf::Vertex line[] = {
			    sf::Vertex(sf::Vector2f(-2, -2) + curr_cell->pos),
			    sf::Vertex(sf::Vector2f(-2, 2) + map->at(curr_cell->indeces[(int)dir]).pos) };

			line[0].color = sf::Color(250, 100, 50);
			line[1].color = sf::Color(250, 100, 50);

			window->draw(line, 2, sf::Lines);
		}
	}
}


namespace{

struct path_cell{
	std::list<uint32_t> path{};
	float weight = INFINITY;
};

float get_path_weight_f(std::vector<cell> &map, uint32_t target_cell_index,
	std::shared_ptr<unit> unit, uint32_t player_index){
	terrain_en ter_type = map[target_cell_index].ter.type;
	if(unit->speed[(int)ter_type] == 0){
		return INFINITY;
	}
	if(map[target_cell_index].player_visible[player_index] == true)
		return 1 / unit->speed[(int)ter_type];
	else
		return 1.5 / unit->speed[(int)terrain_en::PLAIN];
}

void fill_queue_f(std::vector<cell> &map, std::vector<path_cell> &map_path,
	std::list<uint32_t> &index_queue, uint32_t index, std::shared_ptr<unit> unit,
	uint32_t player_index){

	for(auto &next_index : map[index].indeces){
		if(next_index == UINT32_MAX)
			continue;

		float weight = map_path[index].weight + get_path_weight_f(map, next_index,
			unit, player_index);
		if(map_path[next_index].weight > weight){
			std::list<uint32_t> path = map_path[index].path;
			path.emplace_back(next_index);
			map_path[next_index].path = path;
			map_path[next_index].weight = weight;

			index_queue.push_back(next_index);
		}
	}
}

bool infinity_check(game_info *info, std::shared_ptr<unit> unit, uint32_t point){
	terrain_en ter_type = info->get_cell(point).ter.type;
	if(unit->speed[(int)ter_type] == 0)
		return true;
	else
		return false;
}

}

std::list<uint32_t> path_find(game_info *info, uint32_t start_point,
	uint32_t finish_point, std::shared_ptr<unit> unit, uint32_t player_index){

	std::vector<path_cell> map_path{};
	map_path.resize(info->map.size());

	std::list<uint32_t> index_queue{};
	index_queue.push_back(start_point);
	map_path[start_point].weight = 0;
	map_path[start_point].path.emplace_back(start_point);

	if(infinity_check(info, unit, finish_point))
		return map_path[start_point].path;


	uint32_t index;
	do{
		index = index_queue.front();
		index_queue.pop_front();

		fill_queue_f(info->map, map_path, index_queue, index, unit, player_index);
		if(index == finish_point){
			map_path[index].path.pop_front();
		 	return map_path[index].path;
		}
	} while(!index_queue.empty());
}

namespace{

sf::Vector2f perspective_f(sf::Vector2f position, sf::View *view){
	float depth = 1000;
	float transform_rate = (depth - position.y) / depth;
	return sf::Vector2f(
		(position.x - view->getCenter().x) * transform_rate + view->getCenter().x,
		(position.y - view->getCenter().y + depth / 3)
		* transform_rate + view->getCenter().y - depth / 3);
}

sf::Vertex perspective_vertex_f(sf::Vertex shape_vertex,
	sf::Vector2f position, sf::View *view){

	float x_position = shape_vertex.position.x + position.x;
	float y_position = shape_vertex.position.y + position.y;
	return sf::Vertex(perspective_f(sf::Vector2f{x_position, y_position}, view));
}

bool on_screen_f(cell *cell, sf::View *view){
	int cull = view->getSize().x;
	if((cell->pos.x > cull) || (cell->pos.x < -cull))
		return false;
	if((cell->pos.y > cull) || (cell->pos.y < -cull))
		return false;
	return true;
}

sf::Color get_color_in_view(terrain_en terr){
	switch(terr){
		case terrain_en::RIVER :
			return sf::Color(10, 10, 100);
			break;
		case terrain_en::MOUNTAIN :
			return sf::Color(50, 50, 50);
			break;
		case terrain_en::PLAIN :
			return sf::Color(150, 50, 20);
			break;
		case terrain_en::PALM :
			return sf::Color(30, 120, 60);
			break;
	}
}

sf::Color get_color_out_of_view(terrain_en terr){
	switch(terr){
		case terrain_en::RIVER :
			return sf::Color(80, 80, 90);
			break;
		case terrain_en::MOUNTAIN :
			return sf::Color(80, 80, 80);
			break;
		case terrain_en::PLAIN :
			return sf::Color(115, 100, 100);
			break;
		case terrain_en::PALM :
			return sf::Color(100, 100, 80);
			break;
	}
}

sf::Sprite get_sprite_out_of_view(terrain_en terr, sf::Vector2f pos){
	sf::Sprite result(object::textures[(int)object::texture_type::SCHEME]);
	int width = 50;
	int height = 50;
	sf::IntRect rectangle{};
	switch(terr){
		case terrain_en::RIVER :
			rectangle = sf::IntRect{0 + 2 * width, 0, width, height};
		//	frame.setTexture(object::textures[(int)object::texture_type::MOUNTAIN]);
			break;
		case terrain_en::MOUNTAIN :
			rectangle = sf::IntRect{0 + 0 * width, 0, width, height};
			break;
		case terrain_en::PALM :
			rectangle = sf::IntRect{0 + 1 * width, 0, width, height};
			break;
		default:
			rectangle = sf::IntRect{0, 0, 0, 0};
	}
	result.setTextureRect(rectangle);
	result.setOrigin(25, 25);
	result.setScale(0.2, -0.2);
	result.setPosition(pos);
	return result;
}

bool any_of_player_f(std::vector<uint32_t> *visible_players_indeces,
	std::vector<bool> *player_visible){

	for(auto& player_index: *visible_players_indeces){
		if(player_visible->at(player_index))
			return true;
	}
	return false;
}

void create_transform_shape_f(game_info *info, sf::Vector2f pos,
	sf::Vertex *transform_shape, sf::Color color){

	for(uint32_t i = 0; i < 7; ++i){
		transform_shape[i] = perspective_vertex_f(cell::shape[i], pos, &info->view);
		transform_shape[i].color = color;
	}
}

void draw_scheme_map_f(game_info *info){
	for(auto &cell : info->map){
		if(any_of_player_f(&info->visible_players_indeces, &cell.player_visible)
			&& on_screen_f(&cell, &info->view)){

			sf::Vertex transform_shape[7];
			create_transform_shape_f(info, cell.pos, transform_shape, sf::Color::Black);

			sf::ConvexShape polygon;
			polygon.setPointCount(6);
			polygon.setPoint(0, transform_shape[0].position);
			polygon.setPoint(1, transform_shape[1].position);
			polygon.setPoint(2, transform_shape[2].position);
			polygon.setPoint(3, transform_shape[3].position);
			polygon.setPoint(4, transform_shape[4].position);
			polygon.setPoint(5, transform_shape[5].position);
			polygon.setPoint(6, transform_shape[6].position);

			polygon.setFillColor(get_color_out_of_view(cell.ter.type));

			if((transform_shape[0].position.y < transform_shape[1].position.y)){
				info->window.draw(polygon);
				if(info->draw_cells)
					info->window.draw(transform_shape, 7, sf::LineStrip);
				info->window.draw(get_sprite_out_of_view(cell.ter.type,
					perspective_f(cell.pos, &info->view)));
			}
		}
	}
}

void draw_vision_map_f(game_info *info, std::vector<uint32_t> *vision_indeces,
	float time, std::vector<sf::Sprite> *object_sprites){

	for(auto cell_index : *vision_indeces){
		cell &cell = info->map[cell_index];
		if(any_of_player_f(&info->visible_players_indeces, &cell.player_visible)
			&& on_screen_f(&cell, &info->view)){

			sf::Vertex transform_shape[7];
			create_transform_shape_f(info, cell.pos, transform_shape, sf::Color::Black);

			sf::ConvexShape polygon;
			polygon.setPointCount(6);
			polygon.setPoint(0, transform_shape[0].position);
			polygon.setPoint(1, transform_shape[1].position);
			polygon.setPoint(2, transform_shape[2].position);
			polygon.setPoint(3, transform_shape[3].position);
			polygon.setPoint(4, transform_shape[4].position);
			polygon.setPoint(5, transform_shape[5].position);
			polygon.setPoint(6, transform_shape[6].position);

			polygon.setFillColor(get_color_in_view(cell.ter.type));

			if((transform_shape[0].position.y < transform_shape[1].position.y)){
				info->window.draw(polygon);
				if(info->draw_cells)
					info->window.draw(transform_shape, 7, sf::LineStrip);
				for(auto &obj : cell.ter.objects){
					sf::Sprite sprite = obj.update_sprite(time);
					sprite.setPosition(perspective_f(cell.pos + obj.pos, &info->view));
					object_sprites->emplace_back(sprite);
				}
			}
		}
	}
}

void draw_objects(game_info *info, std::vector<sf::Sprite> *object_sprites){
	std::sort(object_sprites->begin(), object_sprites->end(),
		[](sf::Sprite f, sf::Sprite s)
		{ return f.getPosition().y > s.getPosition().y;});

	for(auto &sprite : *object_sprites){
		info->window.draw(sprite);
	}
}

bool is_inside_f(game_info *info, cell *cell, sf::Vector2f pos,
	sf::Vertex *transform_shape){

	create_transform_shape_f(info, cell->pos, transform_shape, sf::Color::White);

	for(uint32_t i = 0; i < 6; ++i){

		float x = pos.x;
		float y = pos.y;
		float x1 = transform_shape[i].position.x;
		float y1 = transform_shape[i].position.y;
		float x2 = transform_shape[i + 1].position.x;
		float y2 = transform_shape[i + 1].position.y;

		float D = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);

		if(D < 0)
			return false;
	}
	return true;
}

void draw_cell_f(game_info *info, sf::Vertex *transform_shape){
	info->window.draw(transform_shape, 7, sf::LineStrip);
}

void draw_selected_cell(game_info *info, uint32_t player_index){
	player &player = info->players[player_index];

	if(player.selected_cell != UINT32_MAX){
		sf::Vertex transform_shape[7];
		create_transform_shape_f(info, info->map[player.selected_cell].pos,
			transform_shape, sf::Color(88, 244, 122));

		info->window.draw(transform_shape, 7, sf::LineStrip);
	}
}

}

void draw_map(game_info *info, float time, uint32_t player_index){

	draw_scheme_map_f(info);

	std::vector<uint32_t> vision_indeces{};
	for(auto &curr_player_index : info->visible_players_indeces){
		for(auto &unit : info->players[curr_player_index].units){
			unit->open_vision(info, curr_player_index);
			std::copy(unit->vision_indeces.begin(), unit->vision_indeces.end(),
				std::back_inserter(vision_indeces));
		}
	}
	std::sort(vision_indeces.begin(), vision_indeces.end());
	std::unique(vision_indeces.begin(), vision_indeces.end());

	std::vector<sf::Sprite> object_sprites{};
	draw_vision_map_f(info, &vision_indeces, time, &object_sprites);

	for(auto &player : info->players){
		for(auto &unit : player.units){
			for(auto &sprite : unit->sprites){
				sprite.setPosition(
					perspective_f(info->map[unit->cell_index].pos + sf::Vector2f{2, -8},
						&info->view));
				object_sprites.emplace_back(sprite);
			}
		}
	}

	sf::Vector2f mouse_pos = mouse_on_map(info);
	for(uint32_t i = 0; i < info->map.size(); ++i){
		if(on_screen_f(&info->map[i], &info->view)){
			sf::Vertex transform_shape[7];
			if(is_inside_f(info, &info->map[i], mouse_pos, transform_shape)){
				draw_cell_f(info, transform_shape);
			}
		}
	}

	draw_selected_cell(info, player_index);
	draw_objects(info, &object_sprites);
}

void move_map(std::vector<cell> *map, cardinal_directions_t dir, float speed){
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

	for(auto &cell : *map)
		cell.pos += offset;
}

std::vector<cell> generate_world(uint32_t size){
	srand(time(NULL));

	object::fill_textures();
	std::vector<cell> map{};
	map.emplace_back(cell{});

	for(uint32_t i = 0; i < size; ++i){
		generate_level(&map);
	}
	return map;
}

void object::fill_textures(){
	object::textures.resize((int)object::texture_type::SIZE);

	object::textures[(int)object::texture_type::SCHEME].loadFromFile("./../data/ch_map.png");
	object::textures[(int)object::texture_type::PALM].loadFromFile("./../data/palms.png");
	object::textures[(int)object::texture_type::MOUNTAIN].loadFromFile("./../data/mountain.png");
}

game_info::game_info()
	: Width(1400), Height(900), draw_cells(false), speed(X1), pause(true),
	window(sf::VideoMode(Width, Height), "Bong"), view(){

	view.setSize(Width, -Height);
	view.setCenter(0, 0);
	view.zoom(0.2);

	window.setView(view);
	window.setVerticalSyncEnabled(true);

	zoom_manager = 0;
	view_size = view.getSize();
	display_rate = view_size.x / view_size.y;

	const float side_size = Width/48/3;
	cell::set_side_size(side_size);

	map = generate_world(40u);
	unit::fill_textures();
}

uint32_t add_player(game_info *info, std::string name, bool is_visible){
	if(info->map.size() == 0)
		throw std::runtime_error("World not created.");

	player player{};
	if(name != "")
		player.name = name;

	uint32_t player_index = info->players.size();
	if(is_visible)
		info->visible_players_indeces.emplace_back(player_index);
	info->players.emplace_back(player);

	uint32_t players_count = info->players.size();
	std::for_each(info->map.begin(), info->map.end(),
		[&players_count](cell &cell){ cell.player_visible.emplace_back(false); });

	return player_index;
}

namespace{
std::vector<uint32_t> open_adjacent_f(game_info *info, uint32_t player_index,
	uint32_t cell_index, cardinal_directions_t dir, uint32_t depth,
	bool to_left, bool to_right){

	using cd_t = cardinal_directions_t;

	std::vector<uint32_t> dst{};
	dst.emplace_back(cell_index);

	cell *cell = &info->map[cell_index];
	cell->player_visible[player_index] = true;

	if((depth == 0) || (cell->ter.type == terrain_en::MOUNTAIN))
		return dst;
	std::vector<uint32_t> src{};

	uint32_t dir_index = cell->indeces[(int)dir];
	src = open_adjacent_f(info, player_index, dir_index, dir, depth - 1, false, false);
	std::copy(src.begin(), src.end(), std::back_inserter(dst));

	if(to_left){
		cd_t previous_dir = previous(dir);
		dir_index = cell->indeces[(int)previous_dir];
		src = open_adjacent_f(info, player_index, dir_index, previous_dir,
			depth - 1, !to_left, !to_right);
		std::copy(src.begin(), src.end(), std::back_inserter(dst));
	}

	if(to_right){
		cd_t next_dir = next(dir);
		dir_index = cell->indeces[(int)next_dir];
		src = open_adjacent_f(info, player_index, dir_index, next_dir,
			depth - 1, !to_left, !to_right);
		std::copy(src.begin(), src.end(), std::back_inserter(dst));
	}

	return dst;
}

std::vector<uint32_t> open_adjacent_f(game_info *info, uint32_t player_index,
	uint32_t cell_index, uint32_t depth){
	using cd_t = cardinal_directions_t;

	std::vector<uint32_t> dst{};
	dst.emplace_back(cell_index);

	cell *cell = &info->map[cell_index];
	cell->player_visible[player_index] = true;

	for(cd_t dir = cd_t::BEGIN; dir < cd_t::END; dir = (cd_t)((int)dir + 1)){
		uint32_t dir_index = cell->indeces[(int)dir];
		std::vector<uint32_t> src =
			open_adjacent_f(info, player_index, dir_index, dir, depth - 1, false, true);

		std::copy(src.begin(), src.end(), std::back_inserter(dst));
	}

	return dst;
}

}

void unit::open_vision(game_info *info, uint32_t player_index){
	this->vision_indeces = open_adjacent_f(info, player_index,
		this->cell_index, this->vision_range);
}

void player_respawn(game_info *info, uint32_t player_index){
	uint32_t spawn_cell_index = 0;
	info->view.setCenter(info->map[spawn_cell_index].pos);

	std::shared_ptr<unit> caravan =
		unit::create_caravan(unit::weight_level_type::LIGHT, spawn_cell_index);
	info->players[player_index].units.emplace_back(caravan);
}

void unit::fill_textures(){
	unit::textures.resize((int)unit::unit_type::SIZE);

	unit::textures[(int)unit::unit_type::CARAVAN].loadFromFile("./../data/caravan_60x60x8.png");
}

namespace{

sf::Sprite create_sprite_f(sf::Texture *texture,
	int width, int height, int column, int raw){

	sf::Sprite frame;

	sf::IntRect rectangle{0 + width * column, 0 + height * raw,
		 width, height};

	frame.setTexture(*texture);
	frame.setTextureRect(rectangle);
	frame.setOrigin(width / 2, height);
	frame.setScale (0.2, -0.2);
	return frame;
}

}

std::shared_ptr<unit> unit::create_caravan(unit::weight_level_type weight, uint32_t cell_index_){
	unit result{};
	result.vision_range = 3;
	result.cell_index = cell_index_;
	result.sprites.emplace_back(
		create_sprite_f(&unit::textures[(int)unit::unit_type::CARAVAN],
		60, 60, 0, 0));



	switch(weight){
		case unit::weight_level_type::LIGHT :
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[(int)unit::unit_type::CARAVAN],
				60, 60, 2, 0));
			result.speed_mod = (float)2 / 10000;
		break;
		case unit::weight_level_type::LOADED :
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[(int)unit::unit_type::CARAVAN],
				60, 60, 1, 0));
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[(int)unit::unit_type::CARAVAN],
				60, 60, 0, 1));
			result.speed_mod = (float)1 / 10000;
		break;
		case unit::weight_level_type::OVERLOADED :
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[(int)unit::unit_type::CARAVAN],
				60, 60, 1, 0));
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[(int)unit::unit_type::CARAVAN],
				60, 60, 0, 1));
			result.sprites.emplace_back(
				create_sprite_f(&unit::textures[(int)unit::unit_type::CARAVAN],
				60, 60, 1, 1));
			result.speed_mod = (float)0.5 / 10000;
		break;
	}

	result.speed[(int)terrain_en::RIVER] = 0;
	result.speed[(int)terrain_en::MOUNTAIN] = 0.5;
	result.speed[(int)terrain_en::PLAIN] = 2;
	result.speed[(int)terrain_en::PALM] = 2;

	return std::make_shared<unit>(result);
}

sf::Vector2f mouse_on_map(game_info *info){
	sf::Vector2i position
		= sf::Mouse::getPosition() - info->window.getPosition();
	position
		= sf::Vector2i(position.x, info->Height - position.y)
		- sf::Vector2i(info->Width / 2, info->Height / 2);


	sf::Vector2f pos = sf::Vector2f(position.x - 9, position.y + 29);
	sf::Vector2f view_size = info->view.getSize();
	float rate = info->Width / view_size.x;
	return sf::Vector2f(pos.x / rate, pos.y / rate);
}

void draw_path(game_info *info, std::list<uint32_t> path, float progress){
	if(path.size() == 0)
		return ;

	float scale = info->view_size.x / info->Width;

	sf::Vector2f first_point = perspective_f(info->map[path.front()].pos, &info->view);
	path.pop_front();
	sf::Vector2f second_point = perspective_f(info->map[path.front()].pos, &info->view);

	sf::Vector2f dif = second_point - first_point;
	dif *= progress;
	first_point += dif;

	sf::Vertex line[2] = {
		sf::Vertex(first_point),
		sf::Vertex(second_point)
	};

	sf::CircleShape circle{};
	circle.setRadius(5 * scale);
	circle.setOrigin(circle.getRadius(), circle.getRadius());

	while(!path.empty()){
		second_point = perspective_f(info->map[path.front()].pos, &info->view);
		path.pop_front();

		line[1] = sf::Vertex(second_point);
		circle.setPosition(second_point);

		info->window.draw(line, 2, sf::Lines);
		info->window.draw(circle);

		line[0] = line[1];
	}
}

uint32_t get_cell_index_under_mouse(game_info *info){
	sf::Vector2f mouse_pos = mouse_on_map(info);

	uint32_t res = UINT32_MAX;
	for(uint32_t i = 0; i < info->map.size(); ++i){
		if(on_screen_f(&info->map[i], &info->view)){
			sf::Vertex transform_shape[7];
			if(is_inside_f(info, &info->map[i], mouse_pos, transform_shape)){
				res = i;
				break;
			}
		}
	}
	return res;
}

std::list<player::selected_unit_type> units_on_cells(
	game_info *info, std::list<uint32_t> players_indeces, std::list<uint32_t> cells){

	std::list<player::selected_unit_type> result{};

	for(auto &player_index : players_indeces){
		for(auto& unit_ptr: info->players[player_index].units){
			for(auto& cell_index : cells){
				if(cell_index == unit_ptr->cell_index){
					result.emplace_back(player_index, unit_ptr);
				}
			}
		}
	}

	return result;
}

void units_draw_paths(game_info *info, uint32_t player_index){
	auto &selected_units = info->players[player_index].selected_units;

	for(auto &curr_unit : selected_units){
		if((curr_unit.first == player_index) && (curr_unit.second.use_count() != 0)){
			std::shared_ptr<unit> unit_ptr = curr_unit.second.lock();

			auto path = unit_ptr->path;
			path.push_front(unit_ptr->cell_index);
			draw_path(info, path, unit_ptr->path_progress);
		}
	}
}

cell& game_info::get_cell(uint32_t index){
	return this->map[index];
}

void unit::unit_update_move(game_info *info, uint32_t player_index, float time){
	if(!this->path.size())
		return;

	terrain_en terr_type = info->get_cell(this->path.front()).ter.type;
	this->path_progress += time * this->speed_mod * this->speed[(int)terr_type];

	if(this->path_progress > 1){
		this->cell_index = this->path.front();
		this->path.pop_front();
		this->path_progress -= 1;

		if(this->path.size()){
			this->path = path_find(info, this->cell_index,
				this->path.back(), shared_from_this(), player_index);
		}
	}
}


void unit::update(game_info *info, uint32_t player_index, float time){

	this->unit_update_move(info, player_index, time);
}

void game_info::update(float time){
	if(this->pause == true)
		return ;

	float time_mod[3] = {1, 2, 4};
	time *= time_mod[this->speed];

	for(uint32_t player_index = 0 ; player_index < this->players.size(); ++player_index){
		for(auto & curr_unit : this->players[player_index].units){
			curr_unit->update(this, player_index, time);
		}
	}
}
