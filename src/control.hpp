#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

void event_handler(game_info *info, float time, uint32_t player_index);

void change_zoom(game_info *info, int value);
void pause(game_info *info);
void speed_up(game_info *info);
void speed_down(game_info *info);
void show_grid(game_info *info);
#endif
