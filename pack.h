#pragma once
#include "item.h"

Item *pack_obj(byte ch, byte *chp);

//get_item: Pick something out of a pack for a purpose
Item *get_item(char *purpose, int type);

//pack_char: Return which character would address a pack object
int pack_char(Item *obj);

//inventory: List what is in the pack
int inventory(std::list<Item *>& list, int type, char *lstr);

