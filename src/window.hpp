#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SFML/Graphics.hpp>

#include <list>
#include <functional>
#include <memory>

class game_window;

class widget{
public:
	widget(sf::Vector2f position, std::map<std::string, sf::Sprite> *sprites);

	virtual void update(game_window *win) noexcept = 0;
	virtual bool interact(game_window *win, sf::Vector2f pos, sf::Event event) = 0;
	virtual void draw(sf::RenderWindow *window) = 0;

protected:
	std::map<std::string, sf::Sprite> *sprites;
	sf::Vector2f position;
};

class header_bar : public widget{
public:
	header_bar(std::map<std::string, sf::Sprite> *sprites, float length);

	void update(game_window *win) noexcept override;
	bool interact(game_window *win, sf::Vector2f position, sf::Event event) override;
	void draw(sf::RenderWindow *window) override;

	inline void change_length(float length) noexcept {this->size.x = length;};
private:
	sf::Vector2f size;

	sf::RectangleShape main_zone;
	std::function<void()> drag_and_drop;

	bool is_move = false;
	sf::Vector2f old_move_position;
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
};

class game_window{
	static sf::Texture texture;
	static std::map<std::string, sf::Sprite> sprites;

	static void load_sprites();
public:
	game_window(sf::Vector2f position, sf::Vector2f size,
		sf::Color color = sf::Color(167, 167, 167));
	void draw(sf::RenderWindow *window, float scale);
	bool interact(sf::Vector2f pos, sf::Event event);
	inline void move(sf::Vector2f offset) noexcept{ this->position += offset;};

	inline const sf::Vector2f& get_size() const noexcept{return this->size;};
	inline const sf::Vector2f& get_position() const noexcept{return this->position;};
	inline const float& get_scale() const noexcept{return this->scale;};

	inline bool is_close() const noexcept {return this->is_close_m;};
private:
	sf::Vector2f position;
	sf::Vector2f size;
	float scale;

	std::list<std::shared_ptr<widget>> widgets;

	bool is_close_m = false;
};

#endif
