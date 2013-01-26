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

#define TREAS_ROOM  20 //one chance in TREAS_ROOM for a treasure room
#define MAXTREAS  10 //maximum number of treasures in a treasure room
#define MINTREAS  2 //minimum number of treasures in a treasure room
#define MAXTRIES  10 //max number of tries to put down a monster

void new_level()
{
  int rm, i;
  THING *tp;
  coord stairs;

  player.t_flags &= ~ISHELD; //unhold when you go down just in case
  //Monsters only get displayed when you move so start a level by having the poor guy rest. God forbid he lands next to a monster!
  if (level>max_level) max_level = level;

  //Clean things off from last level
  clear_level();
  //Free up the monsters on the last level
  for (tp = mlist; tp!=NULL; tp = next(tp)) free_list(tp->t_pack);
  free_list(mlist);
  //just in case we left some flytraps behind
  f_restor();
  //Throw away stuff left on the previous level (if anything)
  free_list(lvl_obj);
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
  } while (!isfloor(chat(stairs.y, stairs.x)));
  set_chat(stairs.y, stairs.x, STAIRS);
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
      } while (!isfloor(chat(stairs.y, stairs.x)));
      unset_flag(stairs.y, stairs.x, F_REAL);
      set_flag(stairs.y, stairs.x, rnd(NTRAPS));
    }
  }
  do
  {
    rm = rnd_room();
    rnd_pos(&rooms[rm], &hero);
  } while (!(isfloor(chat(hero.y, hero.x)) && (flat(hero.y, hero.x)&F_REAL) && moat(hero.y, hero.x)==NULL));
  mpos = 0;
  enter_room(&hero);
  mvaddch(hero.y, hero.x, PLAYER);
  bcopy(oldpos, hero);
  oldrp = proom;
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
  THING *cur;
  int rm;
  coord tp;

  //Once you have found the amulet, the only way to get new stuff is to go down into the dungeon.
  //This is real unfair - I'm going to allow one thing, that way the poor guy will get some food.
  if (saw_amulet && level<max_level) i = MAXOBJ-1;
  else
  {
    //If he is really deep in the dungeon and he hasn't found the amulet yet, put it somewhere on the ground
    //Check this first so if we are out of memory the guy has a hope of getting the amulet
    if (level>=AMULETLEVEL && !saw_amulet)
    {
      if ((cur = new_item())!=NULL)
      {
        attach(lvl_obj, cur);
        cur->o_hplus = cur->o_dplus = 0;
        cur->o_damage = cur->o_hurldmg = "0d0";
        cur->o_ac = 11;
        cur->o_type = AMULET;
        //Put it somewhere
        do {rm = rnd_room(); rnd_pos(&rooms[rm], &tp);} while (!isfloor(winat(tp.y, tp.x)));
        set_chat(tp.y, tp.x, AMULET);
        bcopy(cur->o_pos, tp);
      }
    }
    //check for treasure rooms, and if so, put it in.
    if (rnd(TREAS_ROOM)==0) treas_room();
  }
  //Do MAXOBJ attempts to put things on a level
  for (; i<MAXOBJ; i++) if (total<MAXITEMS && rnd(100)<35)
  {
    //Pick a new object and link it in the list
    cur = new_thing();
    attach(lvl_obj, cur);
    //Put it somewhere
    do {rm = rnd_room(); rnd_pos(&rooms[rm], &tp);} while (!isfloor(chat(tp.y, tp.x)));
    set_chat(tp.y, tp.x, cur->o_type);
    bcopy(cur->o_pos, tp);
  }
}

//treas_room: Add a treasure room
void treas_room()
{
  int nm;
  THING *tp;
  struct room *rp;
  int spots, num_monst;
  coord mp;

  rp = &rooms[rnd_room()];
  spots = (rp->r_max.y-2)*(rp->r_max.x-2)-MINTREAS;
  if (spots>(MAXTREAS-MINTREAS)) spots = (MAXTREAS-MINTREAS);
  num_monst = nm = rnd(spots)+MINTREAS;
  while (nm-- && total<MAXITEMS)
  {
    do {
      rnd_pos(rp, &mp);
    } while (!isfloor(chat(mp.y, mp.x)));
    tp = new_thing();
    bcopy(tp->o_pos, mp);
    attach(lvl_obj, tp);
    set_chat(mp.y, mp.x, tp->o_type);
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
      if (isfloor(chat(mp.y, mp.x)) && moat(mp.y, mp.x)==NULL) break;
    }
    if (spots!=MAXTRIES)
    {
      if ((tp = new_item())!=NULL)
      {
        new_monster(tp, randmonster(FALSE), &mp);
        if (bailout) debug("treasure rm bailout");
        tp->t_flags |= ISMEAN; //no sloughers in THIS room
        give_pack(tp);
      }
    }
  }
  level--;
}
