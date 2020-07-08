#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <SFML/Graphics.hpp>

struct player_info;
class client;

#include "map.hpp"
#include "gui.hpp"
#include "window.hpp"

#include <list>
#include <functional>
#include <memory>

template<typename bounds_type>
bool is_inside(bounds_type bounds, sf::Vector2f pos);

class item_shape;

class client{
public:
	client(game_info *info, int width_, int height_, uint32_t player_index);

	template<typename drawable_type>
	void set_camera_offset(drawable_type& object,
		sf::Vector2f offset = sf::Vector2f(0, 0)) const noexcept;

	template<typename drawable_type>
	void prepare_to_draw(drawable_type& object) const noexcept;

	void set_camera(sf::Vector2f pos);
	void move_camera(cardinal_directions_t dir, float speed);
	void change_zoom(int value);
	void change_show_grid();
	void control_update(game_info *info, float time);

	sf::RenderWindow& get_window() const{return window; };

	float get_view_scale() const;
	float get_view_width() const;
	float get_view_height() const;
	inline sf::Vector2f get_map_offset() const noexcept {return this->camera_pos;};

	int get_zoom_state() const;
	bool is_draw_cells() const;

	const player_info& get_player_info() const;

	sf::Vector2f perspective(sf::Vector2f position) const;
	bool on_screen(const cell *cell) const;
	bool is_open(const cell *cell) const;
	bool is_visable(uint32_t cell_index) const;

	void draw(game_info *info, float time);

	bool draw_cell(sf::Vertex *transform_shape, cell *cell,
		std::function<sf::Color(terrain_en)> func) const;
	void fill_color_cell(game_info *info, uint32_t cell_index, sf::Color outline_color,
		sf::Color fill_color = sf::Color(255, 255, 255, 0)) const;

	void draw_out_of_view(cell *cell) const;
	void draw_objects(std::vector<sf::Sprite> *object_sprites) const;
	void draw_way(sf::Vertex *line, sf::CircleShape *circle, cell *cell) const;
	void draw_selected_cell(game_info *info) const;
	void draw_label(sf::RectangleShape *label) const;
	void draw_buttons(std::vector< button > *buttons) const;
	void draw_cell_border(sf::Vertex *transform_shape) const;
	void draw_item_shape(const item_shape &shape) const;
	void show_cursor_point();

	bool check_previously_visible(game_info *info, uint32_t cell_index) const noexcept;
	std::vector<uint32_t> get_vision_indeces(game_info *info) const;
	sf::Vector2f mouse_on_map() const;

	inline void set_buttons_gui(std::vector<button> buttons){this->buttons_gui = buttons;};
	inline const std::vector<button>& get_buttons_gui() const noexcept {return this->buttons_gui;};
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

	std::vector<button> buttons_gui;

	std::list<game_window> game_windows;
	bool windows_interact(sf::Event event);
	std::vector<bool> vision_map;
};

sf::Vector2f draw_position(const cell *cell_ptr, const client *client_ptr) noexcept;

template<typename bounds_type>
bool is_inside(bounds_type bounds, sf::Vector2f pos){
	sf::FloatRect rect = bounds.getGlobalBounds();

	if((rect.left <= pos.x) && ((rect.left + rect.width) >= pos.x) &&
		(rect.top <= pos.y) && ((rect.top + rect.height) >= pos.y))
		return true;
	else
		return false;
}

inline bool client::is_visable(uint32_t cell_index) const{
	return vision_map[cell_index]; };

template<typename drawable_type>
void client::set_camera_offset(drawable_type& object, sf::Vector2f offset) const noexcept{
	object.setPosition(object.getPosition() + offset + camera_pos);
};

template<typename drawable_type>
void client::prepare_to_draw(drawable_type& object) const noexcept{
	object.setPosition(this->perspective(object.getPosition()));
};

#endif
