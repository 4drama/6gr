#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <SFML/Graphics.hpp>

#include "map.hpp"

//void event_handler(game_info *info, float time, client *client);

void pause(game_info *info);
void speed_up(game_info *info);
void speed_down(game_info *info);
void set_speed(game_info *info, game_info::speed_e speed);
void select_cell(game_info *info, uint32_t player_index, client* client);
void select_units(game_info *info, uint32_t player_index);

#endif
