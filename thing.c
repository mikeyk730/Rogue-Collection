#include <stdlib.h>

#include "rogue.h"
#include "thing.h"
#include "io.h"

THING* the_things;
int* thing_slots;

void alloc_things()
{
  the_things = (THING *)malloc(sizeof(THING)*MAXITEMS);
  thing_slots = (int *)malloc(MAXITEMS*sizeof(int));
  memset(the_things, 0, MAXITEMS*sizeof(THING));
  memset(thing_slots, 0, MAXITEMS*sizeof(int));
}

//new_item: Get a new item with a specified size
THING *create_thing()
{
  int i;
  for (i = 0; i < MAXITEMS; i++)
  {
    if (thing_slots[i]==0)
    {
      thing_slots[i] = 1;
      memset(&the_things[i], 0, sizeof(THING));
      return &the_things[i];
    }
  }
  debug("no more things!");
  return NULL;
}

//discard: Free up an item
int discard(THING *item)
{
  int i;

  for (i = 0; i<MAXITEMS; i++)
  {
    if (item==&the_things[i]) {
      --total; 
      thing_slots[i] = 0; 
      return 1;
    }
  }
  return 0;
}
