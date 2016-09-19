#pragma once
#include "item.h"

//init_colors: Initialize the potion color scheme for this time
void init_colors();
const char* get_color(int type);
void init_new_potion(Item* potion);
int get_potion_value(int type);
const char* get_potion_name(int type);
const char* get_potion_guess(int type);
void set_potion_guess(int type, const char* value);
const char* get_inv_name_potion(Item* potion);

//quaff: Quaff a potion from the pack
void quaff();

//invis_on: Turn on the ability to see invisible
void invis_on();

//turn_see: Put on or off seeing monsters on this level
bool turn_see(bool turn_off);
void turn_see_wrapper(int turn_off);

//th_effect: Compute the effect of this potion hitting a monster.
void affect_monster(Item *obj, Agent *monster);

int is_bad_potion(Item* obj);
int does_know_potion(int type);
void discover_potion(int type);
