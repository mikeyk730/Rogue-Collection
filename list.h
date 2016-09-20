#pragma once
#include <list>
#include "item.h"
#include "agent.h"

//_free_list: Throw the whole blamed thing away
void free_item_list(std::list<Item*>& l);

//_free_list: Throw the whole blamed thing away
void free_agent_list(std::list<Agent*>& l);
