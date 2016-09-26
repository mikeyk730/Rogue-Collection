#pragma once
#include "item.h"

Item *pack_obj(byte ch, byte *chp);


//inventory: List what is in the pack
int inventory(std::list<Item *>& list, int type, char *lstr);

//pick_up: Add something to characters pack.
void pick_up(byte ch);

//get_item: Pick something out of a pack for a purpose
Item *get_item(char *purpose, int type);

//pack_char: Return which character would address a pack object
int pack_char(Item *obj);

//pick_up_gold: Add gold to the pack
void pick_up_gold(int value);

//true if player currently has amulet
bool has_amulet();

//true if player ever had amulet
bool had_amulet();

Item* get_current_weapon();
Item* get_current_armor();
Item* get_ring(int hand);

void set_current_weapon(Item* item);
void set_current_armor(Item* item);
void set_ring(int hand, Item* item);
