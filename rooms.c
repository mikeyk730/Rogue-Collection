//Create the layout for the new level
//rooms.c     1.4 (A.I. Design)       12/16/84

#include "rogue.h"
#include "main.h"
#include "rooms.h"
#include "monsters.h"
#include <ctype.h>

#define GOLDGRP  1

//do_rooms: Create rooms and corridors with a connectivity graph
do_rooms()
{
  int i, rm;
  struct room *rp;
  THING *tp;
  int left_out;
  coord top;
  coord bsze;
  coord mp;
  int old_lev;
  int endline;

  endline = maxrow+1;
  old_lev = level;
  //bsze is the maximum room size
  bsze.x = COLS/3;
  bsze.y = endline/3;
  //Clear things for a new level
  for (rp = rooms; rp<&rooms[MAXROOMS]; rp++) rp->r_goldval = rp->r_nexits = rp->r_flags = 0;
  //Put the gone rooms, if any, on the level
  left_out = rnd(4);
  for (i = 0; i<left_out; i++)
  {
    do rp = &rooms[(rm = rnd_room())]; while (rp->r_flags&ISMAZE);
    rp->r_flags |= ISGONE;
#ifdef TEST
    if (rm>2 && ((level>10 && rnd(20)<level-9) || istest()))
#else TEST
    if (rm>2 && level>10 && rnd(20)<level-9)
#endif TEST
    rp->r_flags |= ISMAZE;
  }
  //dig and populate all the rooms on the level
  for (i = 0, rp = rooms; i<MAXROOMS; rp++, i++)
  {
    //Find upper left corner of box that this room goes in
    top.x = (i%3)*bsze.x+1;
    top.y = i/3*bsze.y;
    if (rp->r_flags&ISGONE)
    {
      //If the gone room is a maze room, draw the maze and set the size equal to the maximum possible.
      if (rp->r_flags&ISMAZE) {rp->r_pos.x = top.x; rp->r_pos.y = top.y; draw_maze(rp);}
      else
      {
        //Place a gone room.  Make certain that there is a blank line for passage drawing.
        do
        {
          rp->r_pos.x = top.x+rnd(bsze.x-2)+1;
          rp->r_pos.y = top.y+rnd(bsze.y-2)+1;
          rp->r_max.x = -COLS;
          rp->r_max.x = -endline;
        } while (!(rp->r_pos.y>0 && rp->r_pos.y<endline-1));
      }
      continue;
    }
    if (rnd(10)<(level-1)) rp->r_flags |= ISDARK;
    //Find a place and size for a random room
    do
    {
      rp->r_max.x = rnd(bsze.x-4)+4;
      rp->r_max.y = rnd(bsze.y-4)+4;
      rp->r_pos.x = top.x+rnd(bsze.x-rp->r_max.x);
      rp->r_pos.y = top.y+rnd(bsze.y-rp->r_max.y);
    } while (rp->r_pos.y==0);
    draw_room(rp);
    //Put the gold in
    if ((rnd(2)==0) && (!saw_amulet || (level>=max_level)))
    {
      THING *gold;

      if ((gold = new_item())!=NULL)
      {
        gold->o_goldval = rp->r_goldval = GOLDCALC;
        while (1)
        {
          byte gch;

          rnd_pos(rp, &rp->r_gold);
          gch = chat(rp->r_gold.y, rp->r_gold.x);
          if (isfloor(gch)) break;
        }
        bcopy(gold->o_pos, rp->r_gold);
        gold->o_flags = ISMANY;
        gold->o_group = GOLDGRP;
        gold->o_type = GOLD;
        attach(lvl_obj, gold);
        chat(rp->r_gold.y, rp->r_gold.x) = GOLD;
      }
    }
    //Put the monster in
    if (rnd(100)<(rp->r_goldval>0?80:25))
    {
      if ((tp = new_item())!=NULL)
      {
        byte mch;

        do {rnd_pos(rp, &mp); mch = winat(mp.y, mp.x);} while (!isfloor(mch));
        new_monster(tp, randmonster(FALSE), &mp);
        give_pack(tp);
      }
    }
  }
}

//draw_room: Draw a box around a room and lay down the floor
draw_room(struct room *rp)
{
  int y, x;

  //Here we draw normal rooms, one side at a time
  vert(rp, rp->r_pos.x); //Draw left side
  vert(rp, rp->r_pos.x+rp->r_max.x-1); //Draw right side
  horiz(rp, rp->r_pos.y); //Draw top
  horiz(rp, rp->r_pos.y+rp->r_max.y-1); //Draw bottom
  chat(rp->r_pos.y, rp->r_pos.x) = ULWALL;
  chat(rp->r_pos.y, rp->r_pos.x+rp->r_max.x-1) = URWALL;
  chat(rp->r_pos.y+rp->r_max.y-1, rp->r_pos.x) = LLWALL;
  chat(rp->r_pos.y+rp->r_max.y-1, rp->r_pos.x+rp->r_max.x-1) = LRWALL;
  //Put the floor down
  for (y = rp->r_pos.y+1; y<rp->r_pos.y+rp->r_max.y-1; y++)
  for (x = rp->r_pos.x+1; x<rp->r_pos.x+rp->r_max.x-1; x++)
  chat(y, x) = FLOOR;
}

//vert: Draw a vertical line
vert(struct room *rp, int startx)
{
  int y;

  for (y = rp->r_pos.y+1; y<=rp->r_max.y+rp->r_pos.y-1; y++) chat(y, startx) = VWALL;
}

//horiz: Draw a horizontal line
horiz(struct room *rp, int starty)
{
  int x;

  for (x = rp->r_pos.x; x<=rp->r_pos.x+rp->r_max.x-1; x++) chat(starty, x) = HWALL;
}

//rnd_pos: Pick a random spot in a room
rnd_pos(struct room *rp, coord *cp)
{
  cp->x = rp->r_pos.x+rnd(rp->r_max.x-2)+1;
  cp->y = rp->r_pos.y+rnd(rp->r_max.y-2)+1;
}

//enter_room: Code that is executed whenever you appear in a room
enter_room(coord *cp)
{
  struct room *rp;
  int y, x;
  THING *tp;

  rp = proom = roomin(cp);
  if (bailout || (rp->r_flags&ISGONE && (rp->r_flags&ISMAZE)==0))
  {
#ifdef DEBUG
    msg("in a gone room");
#endif DEBUG
    return;
  }
  door_open(rp);
  if (!(rp->r_flags&ISDARK) && !on(player, ISBLIND) && !(rp->r_flags&ISMAZE))
  for (y = rp->r_pos.y; y<rp->r_max.y+rp->r_pos.y; y++)
  {
    move(y, rp->r_pos.x);
    for (x = rp->r_pos.x; x<rp->r_max.x+rp->r_pos.x; x++)
    {
      //Displaying monsters is all handled in the chase code now
      tp = moat(y, x);
      if (tp==NULL || !see_monst(tp)) addch(chat(y, x));
      else {tp->t_oldch = chat(y,x); addch(tp->t_disguise);}
    }
  }
}

//leave_room: Code for when we exit a room
leave_room(coord *cp)
{
  int y, x;
  struct room *rp;
  byte floor;
  byte ch;

  rp = proom;
  proom = &passages[flat(cp->y, cp->x)&F_PNUM];
  floor = ((rp->r_flags&ISDARK) && !on(player, ISBLIND))?' ':FLOOR;
  if (rp->r_flags&ISMAZE) floor = PASSAGE;
  for (y = rp->r_pos.y+1; y<rp->r_max.y+rp->r_pos.y-1; y++)
  for (x = rp->r_pos.x+1; x<rp->r_max.x+rp->r_pos.x-1; x++)
  switch (ch = mvinch(y, x))
  {
    case ' ': case PASSAGE: case TRAP: case STAIRS:
    break;

    case FLOOR:
      if (floor==' ') addch(' ');
    break;

    default:
      //to check for monster, we have to strip out standout bit
      if (isupper(toascii(ch)))
      if (on(player, SEEMONST)) {standout(); addch(ch); standend(); break;}
      else moat(y, x)->t_oldch = '@';
      addch(floor);
  }
  door_open(rp);
}
