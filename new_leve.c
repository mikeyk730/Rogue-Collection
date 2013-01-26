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

#define TREAS_ROOM  20 //one chance in TREAS_ROOM for a treasure room
#define MAXTREAS  10 //maximum number of treasures in a treasure room
#define MINTREAS  2 //minimum number of treasures in a treasure room
#define MAXTRIES  10 //max number of tries to put down a monster

void new_level()
{
  int rm, i;
  AGENT *tp;
  Coord stairs;

  player.t_flags &= ~ISHELD; //unhold when you go down just in case
  //Monsters only get displayed when you move so start a level by having the poor guy rest. God forbid he lands next to a monster!
  if (level>max_level) max_level = level;

  //Clean things off from last level
  clear_level();
  //Free up the monsters on the last level
  for (tp = mlist; tp!=NULL; tp = next(tp)) 
    free_item_list(&tp->t_pack);
  free_agent_list(&mlist);
  //just in case we left some flytraps behind
  f_restor();
  //Throw away stuff left on the previous level (if anything)
  free_item_list(&lvl_obj);
  do_rooms(); //Draw rooms
  if (max_level==1)
  {
    clear();
  }
  implode();
  status();
  do_passages(); //Draw passages
  no_food++;
  put_things(); //Place objects (if any)
  //Place the staircase down.
  i = 0;
  do
  {
    rm = rnd_room();
    rnd_pos(&rooms[rm], &stairs);
    if (i++>100) {i = 0; seed = srand2();}
  } while (!isfloor(get_tile(stairs.y, stairs.x)));
  set_tile(stairs.y, stairs.x, STAIRS);
  //Place the traps
  if (rnd(10)<level)
  {
    ntraps = rnd(level/4)+1;
    if (ntraps>MAXTRAPS) ntraps = MAXTRAPS;
    i = ntraps;
    while (i--)
    {
      do
      {
        rm = rnd_room();
        rnd_pos(&rooms[rm], &stairs);
      } while (!isfloor(get_tile(stairs.y, stairs.x)));
      unset_flag(stairs.y, stairs.x, F_REAL);
      set_flag(stairs.y, stairs.x, rnd(NTRAPS));
    }
  }
  do
  {
    rm = rnd_room();
    rnd_pos(&rooms[rm], &player.t_pos);
  } while (!(isfloor(get_tile(player.t_pos.y, player.t_pos.x)) && (get_flags(player.t_pos.y, player.t_pos.x)&F_REAL) && monster_at(player.t_pos.y, player.t_pos.x)==NULL));
  mpos = 0;
  enter_room(&player.t_pos);
  mvaddch(player.t_pos.y, player.t_pos.x, PLAYER);
  bcopy(oldpos, player.t_pos);
  oldrp = player.t_room;
  if (on(player, SEEMONST)) turn_see(FALSE);
}

//rnd_room: Pick a room that is really there
int rnd_room()
{
  int rm;

  do rm = rnd(MAXROOMS); while (!((rooms[rm].r_flags&ISGONE)==0 || (rooms[rm].r_flags&ISMAZE)));
  return rm;
}

//put_things: Put potions and scrolls on this level
void put_things()
{
  int i = 0;
  ITEM *cur;
  int rm;
  Coord tp;

  //Once you have found the amulet, the only way to get new stuff is to go down into the dungeon.
  //This is real unfair - I'm going to allow one thing, that way the poor guy will get some food.
  if (saw_amulet && level<max_level) i = MAXOBJ-1;
  else
  {
    //If he is really deep in the dungeon and he hasn't found the amulet yet, put it somewhere on the ground
    //Check this first so if we are out of memory the guy has a hope of getting the amulet
    if (level>=AMULETLEVEL && !saw_amulet)
    {
      if ((cur = create_item())!=NULL)
      {
        attach_item(&lvl_obj, cur);
        cur->hit_plus = cur->damage_plus = 0;
        cur->damage = cur->throw_damage = "0d0";
        cur->armor_class = 11;
        cur->type = AMULET;
        //Put it somewhere
        do {rm = rnd_room(); rnd_pos(&rooms[rm], &tp);} while (!isfloor(display_character(tp.y, tp.x)));
        set_tile(tp.y, tp.x, AMULET);
        bcopy(cur->pos, tp);
      }
    }
    //check for treasure rooms, and if so, put it in.
    if (rnd(TREAS_ROOM)==0) treas_room();
  }
  //Do MAXOBJ attempts to put things on a level
  for (; i<MAXOBJ; i++) {
    if (total_items<MAXITEMS && rnd(100)<35)
    {
      //Pick a new object and link it in the list
      cur = new_item();
      attach_item(&lvl_obj, cur);
      //Put it somewhere
      do {rm = rnd_room(); rnd_pos(&rooms[rm], &tp);} while (!isfloor(get_tile(tp.y, tp.x)));
      set_tile(tp.y, tp.x, cur->type);
      bcopy(cur->pos, tp);
    }
  }
}

//treas_room: Add a treasure room
void treas_room()
{
  int nm;
  ITEM *tp;
  AGENT *ap;
  struct Room *rp;
  int spots, num_monst;
  Coord mp;

  rp = &rooms[rnd_room()];
  spots = (rp->r_max.y-2)*(rp->r_max.x-2)-MINTREAS;
  if (spots>(MAXTREAS-MINTREAS)) spots = (MAXTREAS-MINTREAS);
  num_monst = nm = rnd(spots)+MINTREAS;
  while (nm-- && total_items<MAXITEMS)
  {
    do {
      rnd_pos(rp, &mp);
    } while (!isfloor(get_tile(mp.y, mp.x)));
    tp = new_item();
    bcopy(tp->pos, mp);
    attach_item(&lvl_obj, tp);
    set_tile(mp.y, mp.x, tp->type);
  }
  //fill up room with monsters from the next level down
  if ((nm = rnd(spots)+MINTREAS)<num_monst+2) nm = num_monst+2;
  spots = (rp->r_max.y-2)*(rp->r_max.x-2);
  if (nm>spots) nm = spots;
  level++;
  while (nm--)
  {
    for (spots = 0; spots<MAXTRIES; spots++)
    {
      rnd_pos(rp, &mp);
      if (isfloor(get_tile(mp.y, mp.x)) && monster_at(mp.y, mp.x)==NULL) break;
    }
    if (spots!=MAXTRIES)
    {
      if ((ap = create_agent())!=NULL)
      {
        new_monster(ap, randmonster(FALSE), &mp);
        if (bailout) debug("treasure rm bailout");
        ap->t_flags |= ISMEAN; //no sloughers in THIS room
        give_pack(ap);
      }
    }
  }
  level--;
}
