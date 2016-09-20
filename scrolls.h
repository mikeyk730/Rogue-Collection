#pragma once
#include "item.h"

std::string get_title(int type);
void init_new_scroll(Item* scroll);

void read_scroll();
int is_scare_monster_scroll(Item* item);
int is_bad_scroll(Item* item);
