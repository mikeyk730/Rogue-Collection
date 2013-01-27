#include <stdlib.h>

#include "rogue.h"
#include "thing.h"

ITEM *create_item(int type, int which)
{
  ITEM* item;

  if (total_items >= MAXITEMS)
    return NULL;
  ++total_items;

  item = malloc(sizeof(ITEM));
  memset(item, 0, sizeof(ITEM));
  item->type = type;
  item->which = which;

  return item;
}

void discard_item(ITEM *item)
{
  free(item);
  --total_items;
}

AGENT *create_agent()
{
  AGENT* agent;
  
  if (total_items >= MAXITEMS)
    return NULL;

  agent = malloc(sizeof(AGENT));
  memset(agent, 0, sizeof(AGENT));
  ++total_items;

  return agent;
}

void discard_agent(AGENT *agent)
{
  free(agent);
  --total_items;
}
