#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SFML/Graphics.hpp>

#include <list>
#include <functional>
#include <memory>

template<typename T>
class deferred_deletion_container;

class game_window;

std::shared_ptr<sf::Text> create_text(
	deferred_deletion_container<sf::Text> *text_delete_contaier,
	const sf::String &string, const sf::Font &font, unsigned int characterSize = 20);

class widget{
public:
	widget(sf::Vector2f position, std::map<std::string, sf::Sprite> *sprites);

	virtual void update(game_window *win) noexcept = 0;
	virtual bool interact(game_window *win, sf::Vector2f pos, sf::Event event) = 0;
	virtual void draw(sf::RenderWindow *window) = 0;

protected:
	std::map<std::string, sf::Sprite> *sprites_ptr;
	sf::Vector2f position;
};

class exit_button : public widget{
public:
	exit_button(std::map<std::string, sf::Sprite> *sprites);

	void update(game_window *win) noexcept override;
	bool interact(game_window *win, sf::Vector2f position, sf::Event event) override;
	void draw(sf::RenderWindow *window) override;
private:
	sf::Sprite sprite;
//	std::function<void()> left_click;
};

class header_bar : public widget{
public:
	header_bar(deferred_deletion_container<sf::Text> *text_delete_contaier,
		std::string title, std::map<std::string, sf::Sprite> *sprites, float length);

	void update(game_window *win) noexcept override;
	bool interact(game_window *win, sf::Vector2f position, sf::Event event) override;
	void draw(sf::RenderWindow *window) override;

	inline void change_length(float length) noexcept {this->size.x = length;};
private:
	std::shared_ptr<sf::Text> title_ptr;

	sf::Vector2f size;
	sf::RectangleShape main_zone;

	bool is_move = false;
	sf::Vector2f old_move_position;

	std::vector<sf::Sprite> tmp_sprites;

	exit_button exit_button_m;
};

class window : public widget{
public:
	window(std::map<std::string, sf::Sprite> *sprites, sf::Vector2f size);

	void update(game_window *win) noexcept override;
	bool interact(game_window *win, sf::Vector2f pos, sf::Event event) override;
	void draw(sf::RenderWindow *window) override;

	inline void change_size(sf::Vector2f size_) noexcept {this->size = size_;};
private:
	sf::Vector2f size;

	sf::RectangleShape main_zone;
	std::vector<sf::Sprite> tmp_sprites;
};

class content_box_widget;

class content_box : public widget{
public:
	content_box(std::map<std::string, sf::Sprite> *sprites_,
		sf::Vector2f offset, sf::Vector2f size);

	void update(game_window *win) noexcept override;
	bool interact(game_window *win, sf::Vector2f pos, sf::Event event) override;
	void draw(sf::RenderWindow *window) override;

	inline void add_widget(std::shared_ptr<content_box_widget> widget){
		this->widgets.emplace_back(widget);};

	inline const sf::Vector2f& get_size() const noexcept{return this->size;};
	inline sf::Vector2f get_position() const noexcept{
		return this->position + this->offset;};
	inline const float& get_scale() const noexcept{return this->scale;};

	void refresh(){ widgets.clear(); this->refresh_func(this);};
	void set_refresh_func(std::function<void(content_box*)> refresh_func_) noexcept{
		this->refresh_func = refresh_func_;};
		
private:
	sf::Vector2f offset;
	sf::Vector2f size;
	float scale;

	sf::RectangleShape main_zone;
	std::list<std::shared_ptr<content_box_widget>> widgets;

	std::function<void(content_box*)> refresh_func;
};

class content_box_widget{
public:
	content_box_widget(sf::Vector2f position, std::map<std::string, sf::Sprite> *sprites);

	virtual void update(content_box *box) noexcept = 0;
	virtual bool interact(content_box *box, sf::Vector2f pos, sf::Event event) = 0;
	virtual void draw(sf::RenderWindow *window) = 0;

protected:
	std::map<std::string, sf::Sprite> *sprites_ptr;
	sf::Vector2f position;
};

class game_window{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	static std::map<std::string, sf::Sprite> *get_sprite_ptr(){
		return &game_window::sprites;};

	game_window(deferred_deletion_container<sf::Text> *text_delete_contaier,
		std::string title, sf::Vector2f position, sf::Vector2f size,
		sf::Color color = sf::Color(167, 167, 167));
	void draw(sf::RenderWindow *window, float scale);
	bool interact(sf::Vector2f pos, sf::Event event);
	inline void move(sf::Vector2f offset) noexcept{ this->position += offset;};

	inline const sf::Vector2f& get_size() const noexcept{return this->size;};
	inline const sf::Vector2f& get_position() const noexcept{return this->position;};
	inline const float& get_scale() const noexcept{return this->scale;};

	inline bool is_close() const noexcept {return this->is_close_m;};
	inline void close() noexcept{this->is_close_m = true;};

	inline void add_widget(std::shared_ptr<widget> widget){
		this->widgets.emplace_back(widget);};

private:
	sf::Vector2f position;
	sf::Vector2f size;
	float scale;

	std::list<std::shared_ptr<widget>> widgets;

	bool is_close_m = false;
};

#endif
