#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

void event_handler(game_info *info, float time, uint32_t player_index);

void change_zoom(game_info *info, int value);

#endif
