#pragma once
#include <list>
#include "item.h"
#include "agent.h"

//_detach: Takes an item out of whatever linked list it might be in
void detach_item(std::list<Item*>& l, Item *item);

//_attach: add an item to the head of a list
void attach_item(std::list<Item*>& l, Item *item);

//_free_list: Throw the whole blamed thing away
void free_item_list(std::list<Item*>& l);

//_detach: Takes an agent out of whatever linked list it might be in
void detach_agent(std::list<Agent*>& l, Agent *agent);

//_free_list: Throw the whole blamed thing away
void free_agent_list(std::list<Agent*>& l);
