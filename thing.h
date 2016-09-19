#pragma once
#include "item.h"
#include "agent.h"

//create_thing: Get a new item with a specified size
Item *create_item(int type, int which);
Agent *create_agent();

//discard: Free up an item
void discard_item(Item *item);
void discard_agent(Agent *item);
