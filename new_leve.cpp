//new_level: Dig and draw a new level
//new_level.c 1.4 (A.I. Design) 12/13/84

#include "rogue.h"
#include "new_leve.h"
#include "monsters.h"
#include "potions.h"
#include "main.h"
#include "list.h"
#include "curses.h"
#include "mach_dep.h"
#include "rooms.h"
#include "things.h"
#include "io.h"
#include "passages.h"
#include "misc.h"
#include "level.h"
#include "thing.h"
#include "pack.h"
#include "room.h"

#define TREAS_ROOM  20 //one chance in TREAS_ROOM for a treasure room
#define MAXTREAS  10 //maximum number of treasures in a treasure room
#define MINTREAS  2 //minimum number of treasures in a treasure room
#define MAXTRIES  10 //max number of tries to put down a monster

void new_level(int do_implode)
{
  int i, ntraps;
  Agent *monster;
  Coord pos;

  player.set_is_held(false); //unhold when you go down just in case
  //Monsters only get displayed when you move so start a level by having the poor guy rest. God forbid he lands next to a monster!

  //Clean things off from last level
  clear_level();
  //Free up the monsters on the last level
  for (auto it = level_monsters.begin(); it != level_monsters.end(); ++it){
      monster = *it;
      free_item_list(monster->pack);
  }
  free_agent_list(level_monsters);
  //Throw away stuff left on the previous level (if anything)
  free_item_list(level_items);
  do_rooms(); //Draw rooms
  if (max_level()==1)
  {
    clear();
  }
  if(do_implode) implode();
  status();
  do_passages(); //Draw passages
  no_food++;
  put_things(); //Place objects (if any)
  //Place the staircase down.
  find_empty_location(&pos, false); //TODO: seed used to change after 100 failed attempts
  Level::set_tile(pos, STAIRS);
  //Place the traps
  if (rnd(10)<get_level())
  {
    ntraps = rnd(get_level()/4)+1;
    if (ntraps>MAXTRAPS) ntraps = MAXTRAPS;
    i = ntraps;
    while (i--)
    {
      find_empty_location(&pos, false);
      Level::unset_flag(pos, F_REAL);
      Level::set_flag(pos, rnd(NTRAPS));
    }
  }
  do
  {
    find_empty_location(&player.pos, true);
  } while (!(Level::get_flags(player.pos) & F_REAL));  //don't place hero on a trap
  msg_position = 0;
  enter_room(&player.pos);
  Screen::DrawChar(player.pos, PLAYER);
  oldpos = player.pos;
  oldrp = player.room;
  if (player.detects_others())
      turn_see(false);
}

//put_things: Put potions and scrolls on this level
void put_things()
{
  int i = 0;
  Item *cur;
  Coord tp;

  //Once you have found the amulet, the only way to get new stuff is to go down into the dungeon.
  //This is real unfair - I'm going to allow one thing, that way the poor guy will get some food.
  if (had_amulet() && get_level()<max_level()) i = MAXOBJ-1;
  else
  {
    //If he is really deep in the dungeon and he hasn't found the amulet yet, put it somewhere on the ground
    //Check this first so if we are out of memory the guy has a hope of getting the amulet
    if (get_level()>=AMULETLEVEL && !had_amulet())
    {
        Item* amulet = new Amulet();
        level_items.push_front(cur);

        //Put it somewhere
        find_empty_location(&tp, true);
        Level::set_tile(tp, AMULET);
        amulet->set_location(tp);
    }
    //check for treasure rooms, and if so, put it in.
    if (rnd(TREAS_ROOM)==0) treas_room();
  }
  //Do MAXOBJ attempts to put things on a level
  for (; i<MAXOBJ; i++) {
    if (rnd(100)<35)
    {
      //Pick a new object and link it in the list
      cur = create_item();
      level_items.push_front(cur);
      //Put it somewhere
      find_empty_location(&tp, false);
      Level::set_tile(tp, cur->type);
      cur->pos = tp;
    }
  }
}

//treas_room: Add a treasure room
void treas_room()
{
  int nm;
  Item *item;
  Agent *monster;
  struct Room *room;
  int spots, num_monst;
  Coord pos;

  room = rnd_room();
  spots = (room->size.y-2)*(room->size.x-2)-MINTREAS;
  if (spots>(MAXTREAS-MINTREAS)) spots = (MAXTREAS-MINTREAS);
  num_monst = nm = rnd(spots)+MINTREAS;
  while (nm--)
  {
    do {
      rnd_pos(room, &pos);
    } while (!isfloor(Level::get_tile(pos)));
    item = create_item();
    item->pos = pos;
    level_items.push_front(item);
    Level::set_tile(pos, item->type);
  }
  //fill up room with monsters from the next level down
  if ((nm = rnd(spots)+MINTREAS)<num_monst+2) nm = num_monst+2;
  spots = (room->size.y-2)*(room->size.x-2);
  if (nm>spots) nm = spots;
  while (nm--)
  {
    for (spots = 0; spots<MAXTRIES; spots++)
    {
      rnd_pos(room, &pos);
      if (isfloor(Level::get_tile(pos)) && monster_at(pos)==NULL) break;
    }
    if (spots!=MAXTRIES)
    {
        monster = create_monster(randmonster(false, get_level()+1), &pos, get_level()+1);
        if (invalid_position) 
            debug("treasure roomm bailout");
        monster->set_is_mean(true); //no sloughers in THIS room
        give_pack(monster);
      
    }
  }
}
