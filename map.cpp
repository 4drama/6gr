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

std::vector<uint32_t> path_find(
	std::vector<cell> *map, uint32_t start, uint32_t finish){
	std::vector<uint32_t> result;



/*	if(start == finish){
		result.emplace_back(finish);
		return result;
	}*/
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

}

void draw_map(game_info *info, float time){
	std::vector<sf::Sprite> object_sprites{};

	for(auto &cell : info->map){
		if(on_screen_f(&cell, &info->view)){
			sf::Vertex transform_shape[] = {
				perspective_vertex_f(cell::shape[0], cell.pos, &info->view),
				perspective_vertex_f(cell::shape[1], cell.pos, &info->view),
				perspective_vertex_f(cell::shape[2], cell.pos, &info->view),
				perspective_vertex_f(cell::shape[3], cell.pos, &info->view),
				perspective_vertex_f(cell::shape[4], cell.pos, &info->view),
				perspective_vertex_f(cell::shape[5], cell.pos, &info->view),
				perspective_vertex_f(cell::shape[6], cell.pos, &info->view)
			};

			sf::ConvexShape polygon;
			polygon.setPointCount(6);
			polygon.setPoint(0, transform_shape[0].position);
			polygon.setPoint(1, transform_shape[1].position);
			polygon.setPoint(2, transform_shape[2].position);
			polygon.setPoint(3, transform_shape[3].position);
			polygon.setPoint(4, transform_shape[4].position);
			polygon.setPoint(5, transform_shape[5].position);
			polygon.setPoint(6, transform_shape[6].position);

		//	polygon.setOutlineColor(sf::Color::Red);
		//	polygon.setOutlineThickness(5);
	//		polygon.setPosition(cell.pos);

			switch(cell.ter.type){
				case terrain_en::RIVER : polygon.setFillColor(sf::Color(10, 10, 100));
					break;
				case terrain_en::MOUNTAIN : polygon.setFillColor(sf::Color(50, 50, 50));
					break;
				case terrain_en::PLAIN : polygon.setFillColor(sf::Color(150, 50, 20));
					break;
				case terrain_en::PALM : polygon.setFillColor(sf::Color(40, 130, 10));
					break;
			}

		if((transform_shape[0].position.y < transform_shape[1].position.y)){
			info->window.draw(polygon);
			if(info->draw_cells)
				info->window.draw(transform_shape, 7, sf::LineStrip);
			for(auto &obj : cell.ter.objects){
				sf::Sprite sprite = obj.update_sprite(time);
				sprite.setPosition(perspective_f(cell.pos + obj.pos, &info->view));
				object_sprites.emplace_back(sprite);
			}
		}
	}

//		draw_path(window, &cell, map);
	}

	std::sort(object_sprites.begin(), object_sprites.end(),
		[](sf::Sprite f, sf::Sprite s)
		{ return f.getPosition().y > s.getPosition().y;});

	for(auto &sprite : object_sprites){
		info->window.draw(sprite);
	}
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

	object::textures[(int)object::texture_type::PALM].loadFromFile("palms.png");
	object::textures[(int)object::texture_type::MOUNTAIN].loadFromFile("mountain.png");
}

game_info::game_info()
	: Width(1400), Height(900), draw_cells(false),
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
}
