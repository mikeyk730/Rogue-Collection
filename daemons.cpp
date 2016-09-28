//All the daemon and fuse functions are in here
//@(#)daemons.c       5.1 (Berkeley) 5/11/82

#include "rogue.h"
#include "game_state.h"
#include "daemons.h"
#include "daemon.h"
#include "main.h"
#include "monsters.h"
#include "rooms.h"
#include "io.h"
#include "misc.h"
#include "rip.h"
#include "output_interface.h"
#include "rings.h"
#include "chase.h"
#include "hero.h"
#include "room.h"
#include "level.h"
#include "monster.h"

//doctor: A healing daemon that restores hit points after rest
void doctor()
{
    int lvl = game->hero().stats.level;
    int original_hp = game->hero().get_hp();

    game->turns_since_heal++;

    if (lvl<8)
    {
        if (game->turns_since_heal + (lvl << 1)>20)
            game->hero().increase_hp(1, false, false);
    }
    else if (game->turns_since_heal >= 3)
        game->hero().increase_hp(rnd(lvl - 7) + 1, false, false);

    if (is_ring_on_hand(LEFT, R_REGEN))
        game->hero().increase_hp(1, false, false);
    if (is_ring_on_hand(RIGHT, R_REGEN))
        game->hero().increase_hp(1, false, false);

    if (original_hp != game->hero().get_hp())
    {
        game->turns_since_heal = 0;
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
      create_wandering_monster(); 
      extinguish(rollwand); 
      fuse(swander, 0, WANDER_TIME);
    }
    between = 0;
  }
}

//unconfuse: Release the poor player from his confusion
void unconfuse()
{
  game->hero().set_confused(false);
  msg("you feel less confused now");
}

//unsee: Turn off the ability to see invisible
void unsee()
{
  Monster *th;

  for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it){
      th = *it;
      if (th->is_invisible() && game->hero().can_see_monster(th) && th->oldch != MDK)
          game->screen().mvaddch(th->pos, th->oldch);
  }
  game->hero().set_sees_invisible(false);
}

//sight: He gets his sight back
void sight()
{
  if (game->hero().is_blind())
  {
    extinguish(sight);
    game->hero().set_blind(false);
    if (!game->hero().room->is_gone()) 
      enter_room(&game->hero().pos);
    msg("the veil of darkness lifts");
  }
}

//nohaste: End the hasting
void nohaste()
{
    game->hero().set_is_fast(false);
    msg("you feel yourself slowing down");
}

//stomach: Digest the hero's food
void stomach()
{
    game->hero().digest();
}
