//Code for one creature to chase another
//chase.c     1.32    (A.I. Design) 12/12/84

#include <stdlib.h>

#include "rogue.h"
#include "chase.h"
#include "fight.h"
#include "move.h"
#include "io.h"
#include "sticks.h"
#include "misc.h"
#include "curses.h"
#include "main.h"
#include "monsters.h"
#include "list.h"
#include "level.h"
#include "weapons.h"
#include "scrolls.h"

#define DRAGONSHOT  5 //one chance in DRAGONSHOT that a dragon will flame

Coord ch_ret; //Where chasing takes you

//runners: Make all the running monsters move.
void runners()
{
  AGENT *tp;
  int dist;

  for (tp = mlist; tp!=NULL; tp = next(tp))
  {
    if (!on(*tp, ISHELD) && on(*tp, ISRUN))
    {
      dist = DISTANCE(player.pos.y, player.pos.x, tp->pos.y, tp->pos.x);
      if (!(on(*tp, ISSLOW) || (tp->type=='S' && dist>3)) || tp->turn) do_chase(tp);
      if (on(*tp, ISHASTE)) do_chase(tp);
      dist = DISTANCE(player.pos.y, player.pos.x, tp->pos.y, tp->pos.x);
      if (on(*tp, ISFLY) && dist>3) do_chase(tp);
      tp->turn ^= TRUE;
    }
  }
}

//do_chase: Make one thing chase another.
void do_chase(AGENT *th)
{
  int mindist = 32767, i, dist;
  bool door;
  ITEM *obj;
  struct Room *oroom;
  struct Room *rer, *ree; //room of chaser, room of chasee
  Coord this; //Temporary destination for chaser

  rer = th->room; //Find room of chaser
  if (on(*th, ISGREED) && rer->goldval==0) th->dest = &player.pos; //If gold has been taken, run after hero
  ree = player.room;
  if (th->dest!=&player.pos) ree = roomin(th->dest); //Find room of chasee
  if (ree==NULL) return;
  //We don't count doors as inside rooms for this routine
  door = (get_tile(th->pos.y, th->pos.x)==DOOR);
  //If the object of our desire is in a different room, and we are not in a maze, run to the door nearest to our goal.

over:

  if (rer!=ree && (rer->flags&ISMAZE)==0)
  {
    //loop through doors
    for (i = 0; i<rer->num_exits; i++)
    {
      dist = DISTANCE(th->dest->y, th->dest->x, rer->exits[i].y, rer->exits[i].x);
      if (dist<mindist) {this = rer->exits[i]; mindist = dist;}
    }
    if (door)
    {
      rer = &passages[get_flags(th->pos.y, th->pos.x)&F_PNUM];
      door = FALSE;
      goto over;
    }
  }
  else
  {
    this = *th->dest;
    //For monsters which can fire bolts at the poor hero, we check to see if (a) the hero is on a straight line from it, and (b) that it is within shooting distance, but outside of striking range.
    if ((th->type=='D' || th->type=='I') && (th->pos.y==player.pos.y || th->pos.x==player.pos.x || abs(th->pos.y-player.pos.y)==abs(th->pos.x-player.pos.x)) && ((dist = DISTANCE(th->pos.y, th->pos.x, player.pos.y, player.pos.x))>2 && dist<=BOLT_LENGTH*BOLT_LENGTH) && !on(*th, ISCANC) && rnd(DRAGONSHOT)==0)
    {
      running = FALSE;
      delta.y = sign(player.pos.y-th->pos.y);
      delta.x = sign(player.pos.x-th->pos.x);
      fire_bolt(&th->pos, &delta, th->type=='D'?"flame":"frost");
      return;
    }
  }
  //This now contains what we want to run to this time so we run to it. If we hit it we either want to fight it or stop running
  chase(th, &this);
  if (equal(ch_ret, player.pos)) {attack(th); return;}
  else if (equal(ch_ret, *th->dest))
  {
    for (obj = lvl_obj; obj!=NULL; obj = next(obj)) if (th->dest==&obj->pos)
    {
      byte oldchar;

      detach_item(&lvl_obj, obj);
      attach_item(&th->pack, obj);
      oldchar = (th->room->flags&ISGONE)?PASSAGE:FLOOR;
      set_tile(obj->pos.y, obj->pos.x, oldchar);
      if (cansee(obj->pos.y, obj->pos.x)) mvaddch(obj->pos.y, obj->pos.x, oldchar);
      th->dest = find_dest(th);
      break;
    }
  }
  if (th->type=='F') return;
  //If the chasing thing moved, update the screen
  if (th->oldch!='@')
  {
    if (th->oldch==' ' && cansee(th->pos.y, th->pos.x) && get_tile(th->pos.y, th->pos.x)==FLOOR)
      mvaddch(th->pos.y, th->pos.x, FLOOR);
    else if (th->oldch==FLOOR && !cansee(th->pos.y, th->pos.x) && !on(player, SEEMONST))
      mvaddch(th->pos.y, th->pos.x, ' ');
    else
      mvaddch(th->pos.y, th->pos.x, th->oldch);
  }
  oroom = th->room;
  if (!equal(ch_ret, th->pos))
  {
    if ((th->room = roomin(&ch_ret))==NULL) {th->room = oroom; return;}
    if (oroom!=th->room) th->dest = find_dest(th);
    th->pos = ch_ret;
  }
  if (can_see_monst(th))
  {
    if (get_flags(ch_ret.y, ch_ret.x)&F_PASS) standout();
    th->oldch = mvinch(ch_ret.y, ch_ret.x);
    mvaddch(ch_ret.y, ch_ret.x, th->disguise);
  }
  else if (on(player, SEEMONST))
  {
    standout();
    th->oldch = mvinch(ch_ret.y, ch_ret.x);
    mvaddch(ch_ret.y, ch_ret.x, th->type);
  }
  else th->oldch = '@';
  if (th->oldch==FLOOR && oroom->flags&ISDARK) th->oldch = ' ';
  standend();
}

//see_monst: Return TRUE if the hero can see the monster

int can_see_monst(AGENT *mp)
{
  if (on(player, ISBLIND)) return FALSE;
  if (on(*mp, ISINVIS) && !on(player, CANSEE)) return FALSE;
  if (DISTANCE(mp->pos.y, mp->pos.x, player.pos.y, player.pos.x)>=LAMP_DIST && ((mp->room!=player.room || (mp->room->flags&ISDARK) || (mp->room->flags&ISMAZE)))) return FALSE;
  //If we are seeing the enemy of a vorpally enchanted weapon for the first time, give the player a hint as to what that weapon is good for.
  if (cur_weapon!=NULL && mp->type==cur_weapon->enemy && ((cur_weapon->flags&DIDFLASH)==0))
  {
    cur_weapon->flags |= DIDFLASH;
    msg(flash, get_weapon_name(cur_weapon->which), terse || expert?"":intense);
  }
  return TRUE;
}

//start_run: Set a monster running after something or stop it from running (for when it dies)
void start_run(Coord *runner)
{
  AGENT *tp;

  //If we couldn't find him, something is funny
  tp = monster_at(runner->y, runner->x);
  if (tp!=NULL)
  {
    //Start the beastie running
    tp->flags |= ISRUN;
    tp->flags &= ~ISHELD;
    tp->dest = find_dest(tp);
  }
  else debug("start_run: moat == NULL ???");
}

//chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns TRUE if we want to keep on chasing later. FALSE if we reach the goal.
void chase(AGENT *tp, Coord *ee)
{
  int x, y;
  int dist, thisdist;
  ITEM *obj;
  Coord *er;
  byte ch;
  int plcnt = 1;

  er = &tp->pos;
  //If the thing is confused, let it move randomly. Phantoms are slightly confused all of the time, and bats are quite confused all the time
  if ((on(*tp, ISHUH) && rnd(5)!=0) || (tp->type=='P' && rnd(5)==0) || (tp->type=='B' && rnd(2)==0))
  {
    //get a valid random move
    rndmove(tp, &ch_ret);
    dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);
    //Small chance that it will become un-confused
    if (rnd(30)==17) tp->flags &= ~ISHUH;
  }
  //Otherwise, find the empty spot next to the chaser that is closest to the chasee.
  else
  {
    int ey, ex;

    //This will eventually hold where we move to get closer. If we can't find an empty spot, we stay where we are.
    dist = DISTANCE(er->y, er->x, ee->y, ee->x);
    ch_ret = *er;
    ey = er->y+1;
    ex = er->x+1;
    for (x = er->x-1; x<=ex; x++)
    {
      for (y = er->y-1; y<=ey; y++)
      {
        Coord tryp;

        tryp.x = x;
        tryp.y = y;
        if (offmap(y, x) || !diag_ok(er, &tryp)) continue;
        ch = display_character(y, x);
        if (step_ok(ch))
        {
          //If it is a scroll, it might be a scare monster scroll so we need to look it up to see what type it is.
          if (ch==SCROLL)
          {
            for (obj = lvl_obj; obj!=NULL; obj = next(obj))
            {
              if (y==obj->pos.y && x==obj->pos.x) break;
            }
            if (is_scare_monster_scroll(obj)) continue;
          }
          //If we didn't find any scrolls at this place or it wasn't a scare scroll, then this place counts
          thisdist = DISTANCE(y, x, ee->y, ee->x);
          if (thisdist<dist) {plcnt = 1; ch_ret = tryp; dist = thisdist;}
          else if (thisdist==dist && rnd(++plcnt)==0) {ch_ret = tryp; dist = thisdist;}
        }
      }
    }
  }
}

//roomin: Find what room some coordinates are in. NULL means they aren't in any room.
struct Room *roomin(Coord *cp)
{
  struct Room *rp;
  byte fp;

  for (rp = rooms; rp<=&rooms[MAXROOMS-1]; rp++) 
    if (cp->x<rp->pos.x+rp->size.x && rp->pos.x<=cp->x && cp->y<rp->pos.y+rp->size.y && rp->pos.y<=cp->y) 
      return rp;

  fp = get_flags(cp->y, cp->x);
  if (fp&F_PASS)
    return &passages[fp&F_PNUM];

  debug("in some bizarre place (%d, %d)", cp->y, cp->x);
  bailout++;
  return NULL;
}

//diag_ok: Check to see if the move is legal if it is diagonal
int diag_ok( Coord *sp, Coord *ep )
{
  if (ep->x==sp->x || ep->y==sp->y) return TRUE;
  return (step_ok(get_tile(ep->y, sp->x)) && step_ok(get_tile(sp->y, ep->x)));
}

//cansee: Returns true if the hero can see a certain coordinate.
int cansee(int y, int x)
{
  struct Room *rer;
  Coord tp;

  if (on(player, ISBLIND)) return FALSE;
  if (DISTANCE(y, x, player.pos.y, player.pos.x)<LAMP_DIST) return TRUE;
  //We can only see if the hero in the same room as the coordinate and the room is lit or if it is close.
  tp.y = y;
  tp.x = x;
  rer = roomin(&tp);
  return (rer==player.room && !(rer->flags&ISDARK));
}

//find_dest: find the proper destination for the monster
Coord *find_dest(AGENT *tp)
{
  ITEM *obj;
  int prob;
  struct Room *rp;

  if ((prob = get_monster_carry_prob(tp->type)) <= 0 || tp->room == player.room || can_see_monst(tp)) 
    return &player.pos;
  rp = tp->room;
  for (obj = lvl_obj; obj!=NULL; obj = next(obj))
  {
    if (is_scare_monster_scroll(obj)) continue;
    if (roomin(&obj->pos)==rp && rnd(100)<prob)
    {
      for (tp = mlist; tp!=NULL; tp = next(tp)) if (tp->dest==&obj->pos) break;
      if (tp==NULL) return &obj->pos;
    }
  }
  return &player.pos;
}
