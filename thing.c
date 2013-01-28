#include <stdlib.h>

#include "rogue.h"
#include "thing.h"

ITEM *create_item(int type, int which)
{
  ITEM* item = malloc(sizeof(ITEM));
  memset(item, 0, sizeof(ITEM));

  item->type = type;
  item->which = which;
  item->hit_plus = 0;
  item->damage_plus = 0;
  item->damage = "0d0";
  item->throw_damage = "0d0";
  item->armor_class = 11;
  item->count = 1;
  item->group = 0;
  item->flags = 0;
  item->enemy = 0;

  return item;
}

void discard_item(ITEM *item)
{
  free(item);
}

AGENT *create_agent()
{
  AGENT* agent = malloc(sizeof(AGENT));
  memset(agent, 0, sizeof(AGENT));

  return agent;
}

void discard_agent(AGENT *agent)
{
  free(agent);
}
