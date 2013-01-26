//Routines dealing specifically with rings
//rings.c     1.4 (AI Design) 12/13/84

#include "rogue.h"
#include "rings.h"
#include "pack.h"
#include "misc.h"
#include "potions.h"
#include "main.h"
#include "io.h"
#include "things.h"
#include "mach_dep.h"
#include "weapons.h"

//ring_on: Put a ring on a hand
void ring_on()
{
  THING *obj;
  int ring = -1;

  if ((obj = get_item("put on", RING))==NULL) goto no_ring;
  //Make certain that it is something that we want to wear
  if (obj->o_type!=RING) {msg("you can't put that on your finger"); goto no_ring;}
  //find out which hand to put it on
  if (is_current(obj)) goto no_ring;
  if (cur_ring[LEFT]==NULL) ring = LEFT;
  if (cur_ring[RIGHT]==NULL) ring = RIGHT;
  if (cur_ring[LEFT]==NULL && cur_ring[RIGHT]==NULL) if ((ring = gethand())<0) goto no_ring;
  if (ring<0) {msg("you already have a ring on each hand"); goto no_ring;}
  cur_ring[ring] = obj;
  //Calculate the effect it has on the poor guy.
  switch (obj->o_which)
  {
  case R_ADDSTR: chg_str(obj->o_ac); break;
  case R_SEEINVIS: invis_on(); break;
  case R_AGGR: aggravate(); break;
  }
  msg("%swearing %s (%c)", noterse("you are now "), inv_name(obj, TRUE), pack_char(obj));
  return;
no_ring:
  after = FALSE;
  return;
}

//ring_off: Take off a ring
void ring_off()
{
  int ring;
  THING *obj;
  char packchar;

  if (cur_ring[LEFT]==NULL && cur_ring[RIGHT]==NULL) {msg("you aren't wearing any rings"); after = FALSE; return;}
  else if (cur_ring[LEFT]==NULL) ring = RIGHT;
  else if (cur_ring[RIGHT]==NULL) ring = LEFT;
  else if ((ring = gethand())<0) return;
  mpos = 0;
  obj = cur_ring[ring];
  if (obj==NULL) {msg("not wearing such a ring"); after = FALSE; return;}
  packchar = pack_char(obj);
  if (can_drop(obj)) msg("was wearing %s(%c)", inv_name(obj, TRUE), packchar);
}

//gethand: Which hand is the hero interested in?
int gethand()
{
  int c;

  for (;;)
  {
    msg("left hand or right hand? ");
    if ((c = readchar())==ESCAPE) {after = FALSE; return -1;}
    mpos = 0;
    if (c=='l' || c=='L') return LEFT;
    else if (c=='r' || c=='R') return RIGHT;
    msg("please type L or R");
  }
}

//ring_eat: How much food does this ring use up?
int ring_eat(int hand)
{
  if (cur_ring[hand]==NULL) return 0;
  switch (cur_ring[hand]->o_which)
  {
  case R_REGEN: return 2;
  case R_SUSTSTR: case R_SUSTARM: case R_PROTECT: case R_ADDSTR: case R_STEALTH: return 1;
  case R_SEARCH: return (rnd(5)==0);
  case R_ADDHIT: case R_ADDDAM: return (rnd(3)==0);
  case R_DIGEST: return -rnd(2);
  case R_SEEINVIS: return (rnd(5)==0);
  default: return 0;
  }
}

//ring_num: Print ring bonuses
char *ring_num(THING *obj)
{
  extern char *ring_buf;

  if (!(obj->o_flags&ISKNOW) && !wizard) return "";
  switch (obj->o_which)
  {
  case R_PROTECT: case R_ADDSTR: case R_ADDDAM: case R_ADDHIT:
    ring_buf[0] = ' ';
    strcpy(&ring_buf[1], num(obj->o_ac, 0, RING));
    break;

  default: return "";
  }
  return ring_buf;
}

int is_ring_on_hand(int hand, int ring)
{
  return (cur_ring[hand] != NULL && cur_ring[hand]->o_which == ring);
}

int is_wearing_ring(int ring)
{
  return (is_ring_on_hand(LEFT, ring) || is_ring_on_hand(RIGHT, ring));
}
