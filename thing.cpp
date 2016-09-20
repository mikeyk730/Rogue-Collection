#include <stdlib.h>

#include "rogue.h"
#include "thing.h"
#include "agent.h"
#include "item.h"


Item *create_item(int type, int which)
{
    Item* item = new Item;
    memset(item, 0, sizeof(Item));

    item->initialize(type, which); //todo:ctor

    return item;
}

void discard_item(Item *item)
{
    delete item;
}

void discard_agent(Agent *agent)
{
    delete agent;
}
