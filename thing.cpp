#include <stdlib.h>

#include "rogue.h"
#include "thing.h"

void Item::initialize(int type, int which)
{
    this->type = type;
    this->which = which;
    this->hit_plus = 0;
    this->damage_plus = 0;
    this->damage = "0d0";
    this->throw_damage = "0d0";
    this->armor_class = 11;
    this->count = 1;
    this->group = 0;
    this->flags = 0;
    this->enemy = 0;
}

ITEM *create_item(int type, int which)
{
    ITEM* item = new Item;
    memset(item, 0, sizeof(ITEM));

    item->initialize(type, which); //todo:ctor

    return item;
}

void discard_item(ITEM *item)
{
    delete item;
}

Agent *create_agent()
{
    Agent* agent = new Agent;
    memset(agent, 0, sizeof(Agent)-sizeof(std::list<Item*>)); //todo:very hacky

    return agent;
}

void discard_agent(Agent *agent)
{
    delete agent;
}
