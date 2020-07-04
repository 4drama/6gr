#ifndef MATH_HPP
#define MATH_HPP

#include <SFML/Graphics.hpp>
#include <cmath>

struct line{
	float A;
	float B;
	float C;
};

inline line get_line (sf::Vector2f point_1, sf::Vector2f point_2);
inline float distance(line line, sf::Vector2f point);

inline float length(sf::Vector2f vec);
inline sf::Vector2f direction(sf::Vector2f vec);
inline bool in_range(sf::Vector2f min, sf::Vector2f max, sf::Vector2f value);



inline float length(sf::Vector2f vec){
	return std::sqrt(pow(vec.x, 2) + pow(vec.y, 2));
}

inline sf::Vector2f direction(sf::Vector2f vec){
	return vec / length(vec);
}

inline bool in_range(sf::Vector2f min, sf::Vector2f max, sf::Vector2f value){
	constexpr float precision = 0.0001f;
	return ((min.x - precision) <= value.x && value.x <= (max.x + precision)) &&
		((min.y - precision) <= value.y && value.y <= (max.y + precision)) ? true : false;
}

inline line get_line (sf::Vector2f point_1, sf::Vector2f point_2){
	return line{point_1.y - point_2.y, point_2.x - point_1.x,
		(point_1.x * point_2.y) - (point_2.x * point_1.y)};
}

inline float distance(line line, sf::Vector2f point){
	return ((line.A * point.x) + (line.B * point.y) + line.C)
		/ (std::sqrt(pow(line.A, 2) + pow(line.B, 2)));
}

#endif
