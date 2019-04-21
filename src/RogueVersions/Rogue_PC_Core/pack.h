#pragma once
#include <list>
#include <string>
#include <coord.h>

struct Item;

Item *pack_obj(byte ch, byte *chp);

//get_item: Pick something out of a pack for a purpose
Item* get_item(const std::string& purpose, int type);

//pack_char: Return which character would address a pack object
int pack_char(Item *obj);

//do_call: Allow a user to call a potion, scroll, or ring something
bool do_call();
