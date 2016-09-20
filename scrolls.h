#pragma once
#include "item.h"

std::string get_title(int type);
void init_new_scroll(Item* scroll);
int get_scroll_value(int type);
std::string get_scroll_name(int type);
std::string get_scroll_guess(int type);
void set_scroll_guess(int type, const char* value);
const char* get_inv_name_scroll(Item* scroll);

void read_scroll();

int is_scare_monster_scroll(Item* item);

int is_bad_scroll(Item* item);
int does_know_scroll(int type);
void discover_scroll(int type);
