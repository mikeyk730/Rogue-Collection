#pragma once
#include "item.h"

Item *pack_obj(byte ch, byte *chp);

//get_item: Pick something out of a m_pack for a purpose
Item* get_item(const std::string& purpose, int type);

//pack_char: Return which character would address a m_pack object
int pack_char(Item *obj);

//inventory: List what is in the m_pack
int inventory(std::list<Item *>& list, int type, const char *lstr);

//pick_up: Add something to characters m_pack.
void pick_up(byte ch);

