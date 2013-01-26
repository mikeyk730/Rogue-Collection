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

//doctor: A healing daemon that restores hit points after rest
void doctor()
{
  int lv, ohp;

  lv = player.t_stats.s_lvl;
  ohp = player.t_stats.s_hpt;
  quiet++;
  if (lv<8)
  {
    if (quiet+(lv<<1)>20) player.t_stats.s_hpt++;
  }
  else if (quiet>=3) player.t_stats.s_hpt += rnd(lv-7)+1;
  if (is_ring_on_hand(LEFT, R_REGEN)) player.t_stats.s_hpt++;
  if (is_ring_on_hand(RIGHT, R_REGEN)) player.t_stats.s_hpt++;
  if (ohp!=player.t_stats.s_hpt)
  {
    if (player.t_stats.s_hpt>player.t_stats.s_maxhp) player.t_stats.s_hpt = player.t_stats.s_maxhp;
    quiet = 0;
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

  if (++between>=3+rnd(3))
  {
    if (roll(1, 6)==4) {wanderer(); extinguish(rollwand); fuse(swander, 0, WANDER_TIME);}
    between = 0;
  }
}

//unconfuse: Release the poor player from his confusion
void unconfuse()
{
  player.t_flags &= ~ISHUH;
  msg("you feel less confused now");
}

//unsee: Turn off the ability to see invisible
void unsee()
{
  AGENT *th;

  for (th = mlist; th!=NULL; th = next(th)) if (on(*th, ISINVIS) && can_see_monst(th) && th->t_oldch!='@') mvaddch(th->t_pos.y, th->t_pos.x, th->t_oldch);
  player.t_flags &= ~CANSEE;
}

//sight: He gets his sight back
void sight()
{
  if (on(player, ISBLIND))
  {
    extinguish(sight);
    player.t_flags &= ~ISBLIND;
    if (!(player.t_room->flags & ISGONE)) 
      enter_room(&player.t_pos);
    msg("the veil of darkness lifts");
  }
}

//nohaste: End the hasting
void nohaste()
{
  player.t_flags &= ~ISHASTE;
  msg("you feel yourself slowing down");
}

//stomach: Digest the hero's food
void stomach()
{
  int oldfood, deltafood;

  if (food_left<=0)
  {
    if (food_left--<-STARVE_TIME) death('s');
    //the hero is fainting
    if (no_command || rnd(5)!=0) return;
    no_command += rnd(8)+4;
    player.t_flags &= ~ISRUN;
    running = FALSE;
    count = 0;
    hungry_state = 3;
    msg("%syou faint from lack of food", noterse("you feel very weak. "));
  }
  else
  {
    oldfood = food_left;
    //If you are in 40 column mode use food twice as fast (e.g. 3-(80/40) = 1, 3-(40/40) = 2 : pretty gross huh?)
    deltafood = ring_eat(LEFT)+ring_eat(RIGHT)+1;
    if (terse) deltafood *= 2;
    food_left -= deltafood;
    if (food_left<MORE_TIME && oldfood>=MORE_TIME) {hungry_state = 2; msg("you are starting to feel weak");}
    else if (food_left<2*MORE_TIME && oldfood>=2*MORE_TIME) {hungry_state = 1; msg("you are starting to get hungry");}
  }
}
