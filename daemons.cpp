//All the daemon and fuse functions are in here
//@(#)daemons.c       5.1 (Berkeley) 5/11/82

#include "rogue.h"
#include "daemons.h"
#include "daemon.h"
#include "main.h"
#include "monsters.h"
#include "rooms.h"
#include "io.h"
#include "misc.h"
#include "rip.h"
#include "curses.h"
#include "rings.h"
#include "chase.h"
#include "hero.h"

//doctor: A healing daemon that restores hit points after rest
void doctor()
{
  int lv, ohp;

  lv = player.stats.level;
  ohp = player.stats.get_hp();
  quiet++;
  if (lv<8)
  {
    if (quiet+(lv<<1)>20)
        player.stats.increase_hp(1, false, false);
  }
  else if (quiet>=3) 
      player.stats.increase_hp(rnd(lv-7)+1, false, false);
  if (is_ring_on_hand(LEFT, R_REGEN)) 
      player.stats.increase_hp(1, false, false);
  if (is_ring_on_hand(RIGHT, R_REGEN)) 
      player.stats.increase_hp(1, false, false);
  if (ohp!=player.stats.get_hp())
  {
    quiet = 0;//todo:how does this work?
  }
}

//Swander: Called when it is time to start rolling for wandering monsters
void swander()
{
  daemon(rollwand, 0);
}

//rollwand: Called to roll to see if a wandering monster starts up
void rollwand()
{
  static int between = 0;

  if (++between >= 3 + rnd(3))
  {
    if (roll(1, 6) == 4) {
      wanderer(); 
      extinguish(rollwand); 
      fuse(swander, 0, WANDER_TIME);
    }
    between = 0;
  }
}

//unconfuse: Release the poor player from his confusion
void unconfuse()
{
  player.set_confused(false);
  msg("you feel less confused now");
}

//unsee: Turn off the ability to see invisible
void unsee()
{
  AGENT *th;

  for (auto it = level_monsters.begin(); it != level_monsters.end(); ++it){
      th = *it;
      if (th->is_invisible() && can_see_monster(th) && th->oldch != '@')
          mvaddch(th->pos.y, th->pos.x, th->oldch);
  }
  player.set_sees_invisible(false);
}

//sight: He gets his sight back
void sight()
{
  if (player.is_blind())
  {
    extinguish(sight);
    player.set_blind(false);
    if (!player.room->is_gone()) 
      enter_room(&player.pos);
    msg("the veil of darkness lifts");
  }
}

//nohaste: End the hasting
void nohaste()
{
  player.set_is_fast(false);
  msg("you feel yourself slowing down");
}

//stomach: Digest the hero's food
void stomach()
{
  digest();
}
