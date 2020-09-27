#include "map.hpp"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <stdexcept>
#include <cassert>

#include "math.hpp"

const float pi = 3.14159265359;

float cell::side_size = 0;
sf::Vector2f cell::offset[(int)cardinal_directions_t::END] = {};
sf::Vertex cell::shape[7] = {};
uint32_t terrain::gen_rate[(int)terrain_en::END][(int)terrain_en::END] = {};
std::vector<sf::Texture> object::textures{};

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

cardinal_directions_t get_direction(sf::Vector2f vec){
	using cd_t = cardinal_directions_t;
	sf::Vector2f dir = direction(vec);
	if(in_range(sf::Vector2f(0.866025f, -0.5f), sf::Vector2f(1.0f, 0.5f), dir)){
		return cd_t::EAST;
	} else if(in_range(sf::Vector2f(0.0f, -1.0f), sf::Vector2f(0.866025f, -0.5f), dir)){
		return cd_t::EAST_SOUTH;
	} else if(in_range(sf::Vector2f(-0.866025f, -1.0f), sf::Vector2f(0.0f, -0.5f), dir)){
		return cd_t::WEST_SOUTH;
	} else if(in_range(sf::Vector2f(-1.0f, -0.5f), sf::Vector2f(-0.866025f, 0.5f), dir)){
		return cd_t::WEST;
	} else if(in_range(sf::Vector2f(-0.866025f, 0.5f), sf::Vector2f(0.0f, 1.0f), dir)){
		return cd_t::WEST_NORTH;
	} else if(in_range(sf::Vector2f(0.0f, 0.5f), sf::Vector2f(0.866025f, 1.0f), dir)){
		return cd_t::EAST_NORTH;
	} else
		assert(false);
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
/*
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
*/

namespace{

struct path_cell{
	std::list<uint32_t> path{};
	float weight = INFINITY;
};

float get_path_weight_f(std::vector<cell> &map, uint32_t target_cell_index,
	std::shared_ptr<unit> unit, uint32_t player_index, std::vector<bool> *vision_map){
	terrain_en ter_type = map[target_cell_index].ter.type;
	if(!unit)
		return 1;

	if(map[target_cell_index].player_visible[player_index] == true){
		if(unit->get_speed(ter_type) == 0){
			return INFINITY;
		} else if((map[target_cell_index].unit != nullptr)
			&& vision_map->at(target_cell_index)){
			return 99999999;
		}
		return 1 / unit->get_speed(ter_type);
	} else
		return 1.5 / unit->get_speed(terrain_en::PLAIN);
}

void fill_queue_f(std::vector<cell> &map, std::vector<path_cell> &map_path,
	std::list<uint32_t> &index_queue, uint32_t index, std::shared_ptr<unit> unit,
	uint32_t player_index, bool even, std::vector<bool> *vision_map){

	using cd = cardinal_directions_t;

	int start = even ? (int)cd::BEGIN : (int)cd::LAST;
	int finish = even ? (int)cd::END : (int)cd::BEGIN - 1;
	auto step_funct = even ? [](int &i){ ++i;} : [](int &i){ --i;};
	auto cmp = even ? [](int l, int r){ return l < r;} : [](int l, int r){ return l > r;};

	for(int dir = start; cmp(dir, finish); step_funct(dir)){
		uint32_t dir_index = map[index].indeces[dir];
		if(dir_index == UINT32_MAX)
			continue;

		float weight = map_path[index].weight + get_path_weight_f(map, dir_index,
			unit, player_index, vision_map);
		if(map_path[dir_index].weight > weight){
			std::list<uint32_t> path = map_path[index].path;
			path.emplace_back(dir_index);
			map_path[dir_index].path = path;
			map_path[dir_index].weight = weight;

			index_queue.push_back(dir_index);
		}
	}
}

bool infinity_check(game_info *info, std::shared_ptr<unit> unit, uint32_t point){
	terrain_en ter_type = info->get_cell(point).ter.type;
	if((unit != nullptr) && (unit->get_speed(ter_type) == 0))
		return true;
	else
		return false;
}

}

std::list<uint32_t> path_find(game_info *info, uint32_t start_point,
	uint32_t finish_point, std::shared_ptr<unit> unit, uint32_t player_index,
	bool random_dir){

	auto player_info = info->players[player_index].info;
	std::vector<bool> *vision_map =
		get_vision_map(info, player_info->get_vision_players_indeces());

	std::vector<path_cell> map_path{};
	map_path.resize(info->map.size());

	std::list<uint32_t> index_queue{};
	index_queue.push_back(start_point);
	map_path[start_point].weight = 0;
	map_path[start_point].path.emplace_back(start_point);

	if(infinity_check(info, unit, finish_point))
		return map_path[start_point].path;


	uint32_t index;
	bool even = std::rand() % 1;
	do{
		index = index_queue.front();
		index_queue.pop_front();

		fill_queue_f(info->map, map_path, index_queue, index, unit,
			player_index, even, vision_map);
		even = random_dir ? std::rand() % 1 : !even;
		if(index == finish_point){
			map_path[index].path.pop_front();
		 	return map_path[index].path;
		}
	} while(!index_queue.empty());
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

namespace{
	sf::Vertex perspective_vertex_f(sf::Vertex shape_vertex,
		sf::Vector2f position, const client *client);
}

void create_transform_shape(const client *client, sf::Vector2f pos,
	sf::Vertex *transform_shape, sf::Color color){

	for(uint32_t i = 0; i < 7; ++i){
		transform_shape[i] = perspective_vertex_f(cell::shape[i], pos, client);
		transform_shape[i].color = color;
	}
}

namespace{

sf::Vertex perspective_vertex_f(sf::Vertex shape_vertex,
	sf::Vector2f position, const client *client){

	float x_position = shape_vertex.position.x + position.x;
	float y_position = shape_vertex.position.y + position.y;
	return sf::Vertex(client->perspective(sf::Vector2f{x_position, y_position}));
}

sf::Color get_color_in_view_f(terrain_en terr){
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

sf::Color get_color_out_of_view_f(terrain_en terr){
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

void draw_scheme_map_f(game_info *info, client *client){
	for(auto &cell : info->map){
		if(client->is_open(&cell)){

			sf::Vertex transform_shape[7];
			create_transform_shape(client, draw_position(&cell, client),
				transform_shape, sf::Color::Black);

			if(client->draw_cell(transform_shape, &cell, get_color_out_of_view_f))
				client->draw_out_of_view(&cell);
		}
	}
}

void draw_vision_map_f(game_info *info, client *client,
	std::vector<uint32_t> *vision_indeces, float time,
	std::vector<sf::Sprite> *object_sprites){

	for(auto cell_index : *vision_indeces){
		cell &cell = info->map[cell_index];
		if(client->on_screen(&cell)){

			sf::Vertex transform_shape[7];
			create_transform_shape(client, draw_position(&cell, client),
				transform_shape, sf::Color::Black);

			if(client->draw_cell(transform_shape, &cell, get_color_in_view_f)){
				for(auto &obj : cell.ter.objects){
					sf::Sprite sprite = obj.update_sprite(time);
					sprite.setPosition(client->perspective(draw_position(&cell, client) + obj.pos));
					object_sprites->emplace_back(sprite);
				}
				cell.draw(info, client);
			}
		}
	}
}

}

bool is_inside_f(client *client, cell *cell, sf::Vector2f pos,
	sf::Vertex *transform_shape){

	create_transform_shape(client, draw_position(cell, client), transform_shape, sf::Color::White);

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

void prepare_units_sprites(game_info *info, client *client,
	std::vector<sf::Sprite> &object_sprites){
	player_info player_info = client->get_player_info();
	std::vector<bool> *vision_map =
		get_vision_map(info, player_info.get_vision_players_indeces());

	for(auto &player : info->players){
		for(auto &unit : player.units){
			if(vision_map->at(unit->cell_index)
				&& (client->on_screen(&info->map[unit->cell_index]))){

				for(auto &sprite : unit->sprites){
					sprite.setPosition( client->perspective(
						draw_position(&info->map[unit->cell_index], client)
							+ sf::Vector2f{2, -8}));

					object_sprites.emplace_back(sprite);
				}
			}
		}
	}
}

void draw_cell_under_mouse(game_info *info, client *client){
	sf::Vector2f mouse_pos = client->mouse_on_map();

	for(uint32_t i = 0; i < info->map.size(); ++i){
		if(client->on_screen(&info->map[i])){
			sf::Vertex transform_shape[7];
			if(is_inside_f(client, &info->map[i], mouse_pos, transform_shape)){
				client->draw_cell_border(transform_shape);
			}
		}
	}
}

void draw_map(game_info *info, client *client, float time){

	draw_scheme_map_f(info, client);

	std::vector<sf::Sprite> object_sprites{};
	std::vector<uint32_t> vision_indeces = client->get_vision_indeces(info);

	draw_vision_map_f(info, client, &vision_indeces, time, &object_sprites);
	prepare_units_sprites(info, client, object_sprites);

	draw_cell_under_mouse(info, client);
	client->draw_selected_cell(info);

	client->draw_objects(&object_sprites);
}

/*void move_map(std::vector<cell> *map, cardinal_directions_t dir, float speed){
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
}*/

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

game_info::game_info() : speed(X1), pause(true){
	const float side_size = 1400/48/3;
	cell::set_side_size(side_size);

	map = generate_world(40u);
//	unit::fill_textures();
}

player_info::player_info(game_info *info, uint32_t player_) : player(player_){
	this->control_players.emplace_back(player_);
//	this->update(info);
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

std::list<uint32_t> player_info::get_vision_players_indeces() const{
	std::list<uint32_t> vision_players = this->control_players;
	{
		std::list<uint32_t> addition_vision_players = this->alliance_players;
		vision_players.splice(vision_players.end(), addition_vision_players);
	}
	return vision_players;
}

uint32_t player_info::get_index() const{
	return this->player;
}

void player_info::remove_from_all(uint32_t player){
	alliance_players.remove(player);
	enemy_players.remove(player);
}

void player_info::war(game_info *info, uint32_t player){
	this->remove_from_all(player);
	this->enemy_players.push_back(player);
	this->update(info);
}

player::player(game_info *info, uint32_t player_index)
	: info(std::make_shared<player_info>(info, player_index)){
}

uint32_t add_player(game_info *info, std::string name){
	if(info->map.size() == 0)
		throw std::runtime_error("World not created.");

	uint32_t player_index = info->players.size();

	player player(info, player_index);
	if(name != "")
		player.name = name;

	info->players.emplace_back(player);
	for(auto &c_player : info->players)
		c_player.info->update(info);

	uint32_t players_count = info->players.size();
	std::for_each(info->map.begin(), info->map.end(),
		[&players_count](cell &cell){ cell.player_visible.emplace_back(false); });

	return player_index;
}

void game_info::announce_war(uint32_t player_index_1, uint32_t player_index_2){
	players.at(player_index_1).info->war(this, player_index_2);
	players.at(player_index_2).info->war(this, player_index_1);
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
	if(dir_index != UINT32_MAX){
		src = open_adjacent_f(info, player_index, dir_index, dir, depth - 1, false, false);
		std::copy(src.begin(), src.end(), std::back_inserter(dst));
	}

	if(to_left){
		cd_t previous_dir = previous(dir);
		dir_index = cell->indeces[(int)previous_dir];
		if(dir_index != UINT32_MAX){
			src = open_adjacent_f(info, player_index, dir_index, previous_dir,
				depth - 1, !to_left, !to_right);
			std::copy(src.begin(), src.end(), std::back_inserter(dst));
		}
	}

	if(to_right){
		cd_t next_dir = next(dir);
		dir_index = cell->indeces[(int)next_dir];
		if(dir_index != UINT32_MAX){
			src = open_adjacent_f(info, player_index, dir_index, next_dir,
				depth - 1, !to_left, !to_right);
			std::copy(src.begin(), src.end(), std::back_inserter(dst));
		}
	}

	return dst;
}

}

std::vector<uint32_t> open_adjacent(game_info *info, uint32_t player_index,
	uint32_t cell_index, uint32_t depth){
	using cd_t = cardinal_directions_t;

	std::vector<uint32_t> dst{};
	dst.emplace_back(cell_index);

	cell *cell = &info->map[cell_index];
	cell->player_visible[player_index] = true;

	for(cd_t dir = cd_t::BEGIN; dir < cd_t::END; dir = (cd_t)((int)dir + 1)){
		uint32_t dir_index = cell->indeces[(int)dir];

		if(dir_index != UINT32_MAX){
			std::vector<uint32_t> src =
				open_adjacent_f(info, player_index,
				dir_index, dir, depth - 1, false, true);

			std::copy(src.begin(), src.end(), std::back_inserter(dst));
		}
	}

	return dst;
}

uint32_t choose_spawn_cell(game_info *info){
	uint32_t spawn_cell_index = 0;
	do {
		spawn_cell_index = rand() % info->map.size();
	} while(info->map[spawn_cell_index].ter.type != terrain_en::PLAIN);
	return spawn_cell_index;
}

void player_respawn(game_info *info, client *client){
	uint32_t player_index = client->get_player_info().get_index();
	uint32_t spawn_cell_index = choose_spawn_cell(info);
	if(client != nullptr)
		client->set_camera(-info->map[spawn_cell_index].pos);


	std::shared_ptr<mech> mech = mech::create(player_index,
		spawn_cell_index, &info->item_db);
	mech->fill_energy(1000);

	info->map[spawn_cell_index].unit = mech;
	info->players[player_index].units.emplace_back(mech);

	info->players[player_index].garage.put_item((int)items_id::ROCKET_BASE, 99);
	info->players[player_index].garage.put_item((int)items_id::RADIATOR_75, 99);
	info->players[player_index].garage.put_item((int)items_id::ACCUM_50, 99);
	info->players[player_index].garage.put_item((int)items_id::LEGS_BASE, 99);
	info->players[player_index].garage.put_item((int)items_id::ENGINE_BASE, 99);
	info->players[player_index].garage.put_item((int)items_id::TANK_BASE, 99);
	info->players[player_index].garage.put_item((int)items_id::COOLING_BASE, 99);
	info->players[player_index].garage.put_item((int)items_id::RADIATOR_200, 99);
	info->players[player_index].garage.put_item((int)items_id::ACCUM_100, 99);

	for(auto &unit : info->players[player_index].units)
		unit->open_vision(info, player_index);
}


void draw_path(game_info *info, client* client, std::list<uint32_t> path,
	float progress){

	if(path.size() == 0)
		return ;

	sf::Vector2f first_point = client->perspective(draw_position(&info->map[path.front()], client));
	path.pop_front();
	sf::Vector2f second_point = client->perspective(draw_position(&info->map[path.front()], client));

	sf::Vector2f dif = second_point - first_point;
	dif *= progress;
	first_point += dif;

	sf::Vertex line[2] = {
		sf::Vertex(first_point),
		sf::Vertex(second_point)
	};

	sf::CircleShape circle{};
	circle.setRadius(5 * client->get_view_scale());
	circle.setOrigin(circle.getRadius(), circle.getRadius());

	while(!path.empty()){
		cell &cell = info->map[path.front()];
		second_point = client->perspective(draw_position(&cell, client));
		path.pop_front();

		line[1] = sf::Vertex(second_point);
		circle.setPosition(second_point);

		client->draw_way(line, &circle, &cell);

		line[0] = line[1];
	}
}

uint32_t get_cell_under_position(game_info *info, client *client, sf::Vector2f position){
	uint32_t res = UINT32_MAX;
	for(uint32_t i = 0; i < info->map.size(); ++i){
		sf::Vertex transform_shape[7];
		if(is_inside_f(client, &info->map[i], position, transform_shape)){
			res = i;
			break;
		}
	}
	return res;
}

uint32_t get_cell_index_under_mouse(game_info *info, client *client){
	sf::Vector2f mouse_pos = client->mouse_on_map();

	uint32_t res = UINT32_MAX;
	for(uint32_t i = 0; i < info->map.size(); ++i){
		if(client->on_screen(&info->map[i])){
			sf::Vertex transform_shape[7];
			if(is_inside_f(client, &info->map[i], mouse_pos, transform_shape)){
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

void units_draw_paths(game_info *info, client *client){
	uint32_t player_index = client->get_player_info().get_index();
	auto &selected_units = info->players[player_index].selected_units;

	for(auto &curr_unit : selected_units){
		if((curr_unit.first == player_index) && (curr_unit.second.use_count() != 0)){
			std::shared_ptr<unit> unit_ptr = curr_unit.second.lock();

			auto path = unit_ptr->path;
			path.push_front(unit_ptr->cell_index);
			draw_path(info, client, path, unit_ptr->path_progress);
		}
	}
}

cell& game_info::get_cell(uint32_t index){
	return this->map[index];
}

void game_info::update(float time){
	if(this->pause == true)
		return ;

	for(uint32_t player_index = 0 ; player_index < this->players.size(); ++player_index){
		for(auto & curr_unit : this->players[player_index].units){
			curr_unit->update(this, player_index, time);
		}
	}

	for(auto it = projectiles.begin(); it != projectiles.end(); it++){
		const std::shared_ptr<projectile>& projectile_ptr = *it;
		projectile_ptr->update(this, time);
		if(projectile_ptr->is_explosion()){
		/*	std::list<uint32_t> area = get_area(this, projectile_ptr->get_cell_index(),
				projectile_ptr->get_aoe());*/
			projectile_ptr->detonate(this);
			it = --projectiles.erase(it);
		}
	}

	for(auto it = effects.begin(); it != effects.end(); it++){
		const std::shared_ptr<effect>& effect_ptr = *it;
		effect_ptr->update(time);
		if(!effect_ptr->is_life()){
			it = --effects.erase(it);
		}
	}
}

std::vector<bool>* get_vision_map(game_info *info, std::list<uint32_t> players_indeces){
	static std::vector<bool> map{};
	map.assign(info->map.size(), false);

	for(auto &curr_player_index : players_indeces){
		for(auto &unit : info->players[curr_player_index].units){
			for(auto &vision_index : unit->vision_indeces){
				map[vision_index] = true;
			}
		}
	}
	return &map;
}

std::list<uint32_t> get_path(game_info *info,
	uint32_t start_cell_index, uint32_t target_cell_index, uint32_t depth){
	using cd_t = cardinal_directions_t;

	const cell *start_cell = &info->get_cell(start_cell_index);
	const cell *target_cell = &info->get_cell(target_cell_index);

	const sf::Vector2f &from_point = start_cell->pos;
	const sf::Vector2f &to_point = target_cell->pos;

	line line = get_line(from_point, to_point);

	auto nearest = [to_point, info, &line](const cell *from_cell) -> uint32_t{
		uint32_t nearest_cell_index = UINT32_MAX;
		float range = length(to_point - from_cell->pos);

		for(cd_t dir = cd_t::BEGIN; dir != cd_t::END; dir = cd_t((int)dir + 1)){
			uint32_t current_cell_index = from_cell->indeces[(int)dir];
			if(current_cell_index == UINT32_MAX)
				continue;
			constexpr float precision = 0.0001f;

			float current_distance =
				std::abs(distance(line,  info->get_cell(current_cell_index).pos));
			float nearest_distance = nearest_cell_index != UINT32_MAX ?
				std::abs(distance(line,  info->get_cell(nearest_cell_index).pos)) :
				std::numeric_limits<float>::max();

			float current_range =
				length(to_point - info->get_cell(current_cell_index).pos);

			if((current_range < range) && (current_distance < (nearest_distance + precision))){

				nearest_cell_index = current_cell_index;
			}
		}
		assert(nearest_cell_index != UINT32_MAX);
		return nearest_cell_index;
	};

	std::list<uint32_t> path{start_cell_index};
	for(uint32_t i = 0; (i < depth) && (path.back() != target_cell_index); ++i){
		path.emplace_back(nearest(&info->get_cell(path.back())));
	}

	return path;
}

/*std::list<uint32_t> get_area(game_info *info, uint32_t cell_index, uint32_t depth,
	bool is_root, cardinal_directions_t main_dir){
	using cd_t = cardinal_directions_t;

	std::list<uint32_t> res{cell_index};
	if(depth != 0){
		const cell &curr_cell = info->get_cell(cell_index);
		for(cd_t dir = is_root ? cd_t::BEGIN : previous(main_dir);
			dir != (is_root ? cd_t::END : next(main_dir));
			dir = is_root ? cd_t((int)dir + 1) : next(dir)){

			if(curr_cell.indeces[(int)dir] != UINT32_MAX){
				auto part = get_area(info, curr_cell.indeces[(int)dir], depth - 1, false, dir);
				res.merge(part);
			}
		}
	}

	res.sort();
	res.unique();
	return res;
}*/

namespace{

get_area_f(std::vector<std::list<uint32_t>> &cells, game_info *info,
	uint32_t cell_index, uint32_t depth){
	using cd_t = cardinal_directions_t;

	cells = std::vector<std::list<uint32_t>>(depth + 1);
	cells[0].emplace_back(cell_index);

	for(cd_t dir = cd_t::BEGIN; dir != cd_t::END ; dir = (cd_t)((int)dir + 1)){
		uint32_t current_cell = cell_index;
		if(current_cell == UINT32_MAX){
			continue;
		}
		for(uint32_t current_depth = 1; current_depth < depth + 1; ++current_depth){
			current_cell = info->get_cell(current_cell).indeces[(int)dir];
			if(current_cell == UINT32_MAX){
				break;
			}
			cells[current_depth].emplace_back(current_cell);

			uint32_t l_cell = info->get_cell(current_cell).indeces[(int)previous(dir)];
			for(uint32_t i = current_depth; i < depth; ++i){
				if(l_cell == UINT32_MAX){
					break;
				}
				cells[i + 1].emplace_back(l_cell);
				l_cell = info->get_cell(l_cell).indeces[(int)previous(dir)];
			}
		}
	}
}

}

area::area(game_info *info, uint32_t cell_index, uint32_t depth){
	get_area_f(this->cells, info, cell_index, depth);
}

std::list<uint32_t> area::combine(uint32_t start_lvl, uint32_t end_lvl){
	std::list<uint32_t> res{};
	end_lvl = end_lvl != UINT32_MAX ? end_lvl : this->cells.size();
	for(uint32_t i = start_lvl; i < end_lvl; ++i){
		res.insert(res.end(), this->cells[i].begin(), this->cells[i].end());
	}
	return res;
}

void cell::draw(game_info *info, client *client){
	if(deform){
		deform->draw(info, client);
	}
}

void cell::add_crater(game_info *info, uint32_t cell_index) noexcept{
	if(ter.type != terrain_en::RIVER){
		this->deform = create_crater(info, cell_index);
	}
	this->ter.objects.clear();
	if(ter.type == terrain_en::MOUNTAIN){
		this->ter = terrain{terrain_en::PLAIN};
		info->reopen_vision();
	}
};

void game_info::reopen_vision(){
	for(auto& player : players){
		for(auto& unit : player.units){
			for(auto player_index : player.info->alliance_players){
				unit->open_vision(this, player_index);
			}
			for(auto player_index : player.info->control_players){
				unit->open_vision(this, player_index);
			}
		}
	}
}
