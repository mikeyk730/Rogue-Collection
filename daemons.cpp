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

  lv = pstats.s_lvl;
  ohp = pstats.s_hpt;
  quiet++;
  if (lv<8)
  {
    if (quiet+(lv<<1)>20) pstats.s_hpt++;
  }
  else if (quiet>=3) pstats.s_hpt += rnd(lv-7)+1;
  if (ISRING(LEFT, R_REGEN)) pstats.s_hpt++;
  if (ISRING(RIGHT, R_REGEN)) pstats.s_hpt++;
  if (ohp!=pstats.s_hpt)
  {
    if (pstats.s_hpt>max_hp) pstats.s_hpt = max_hp;
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
    if (roll(1, 6)==4) {wanderer(); extinguish(rollwand); fuse(swander, 0, WANDERTIME);}
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
  THING *th;

  for (th = mlist; th!=NULL; th = next(th)) if (on(*th, ISINVIS) && see_monst(th) && th->t_oldch!='@') mvaddch(th->t_pos.y, th->t_pos.x, th->t_oldch);
  player.t_flags &= ~CANSEE;
}

//sight: He gets his sight back
void sight()
{
  if (on(player, ISBLIND))
  {
    extinguish(sight);
    player.t_flags &= ~ISBLIND;
    if (!(proom->r_flags&ISGONE)) enter_room(&hero);
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
    if (food_left--<-STARVETIME) death('s');
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
    if (food_left<MORETIME && oldfood>=MORETIME) {hungry_state = 2; msg("you are starting to feel weak");}
    else if (food_left<2*MORETIME && oldfood>=2*MORETIME) {hungry_state = 1; msg("you are starting to get hungry");}
  }
}
