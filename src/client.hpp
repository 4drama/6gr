#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <SFML/Graphics.hpp>

struct player_info;
class client;

#include "map.hpp"
#include "gui.hpp"

#include <list>
#include <functional>
#include <memory>

class client{
public:
	client(game_info *info, int width_, int height_, uint32_t player_index);

	void set_camera(sf::Vector2f pos);
	void move_camera(cardinal_directions_t dir, float speed);
	void change_zoom(int value);
	void change_show_grid();
	void control_update(game_info *info, float time);

	float get_view_scale() const;
	float get_view_width() const;
	float get_view_height() const;
	inline sf::Vector2f get_map_offset() const noexcept {return this->camera_pos;};

	int get_zoom_state() const;
	bool is_draw_cells() const;

	const player_info& get_player_info() const;

	sf::Vector2f perspective(sf::Vector2f position) const;
	bool on_screen(cell *cell) const;
	bool is_visable(cell *cell) const;

	void draw(game_info *info, float time);

	bool draw_cell(sf::Vertex *transform_shape, cell *cell,
		std::function<sf::Color(terrain_en)> func) const;
	void draw_out_of_view(cell *cell) const;
	void draw_objects(std::vector<sf::Sprite> *object_sprites) const;
	void draw_way(sf::Vertex *line, sf::CircleShape *circle, cell *cell) const;
	void draw_selected_cell(game_info *info) const;
	void draw_label(sf::RectangleShape *label) const;
	void draw_buttons(std::vector< button > *buttons) const;
	void draw_cell_border(sf::Vertex *transform_shape) const;
	void show_cursor_point();

	std::vector<uint32_t> get_vision_indeces(game_info *info) const;
	sf::Vector2f mouse_on_map() const;

private:
	int Width, Height;

	mutable sf::RenderWindow window;
	sf::View view;
	sf::Vector2f camera_pos;

	sf::Vector2f view_size;
	float display_rate;

	bool draw_cells;
	int zoom_manager;

	std::shared_ptr<player_info> player;
};

sf::Vector2f draw_position(const cell *cell_ptr, const client *client_ptr) noexcept;

#endif
