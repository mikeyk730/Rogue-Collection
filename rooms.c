//Create the layout for the new level
//rooms.c     1.4 (A.I. Design)       12/16/84

#include <ctype.h>

#include "rogue.h"
#include "main.h"
#include "rooms.h"
#include "monsters.h"
#include "list.h"
#include "curses.h"
#include "new_leve.h"
#include "maze.h"
#include "move.h"
#include "chase.h"
#include "misc.h"
#include "io.h"
#include "level.h"
#include "thing.h"
#include "pack.h"

#define GOLDGRP  1

//do_rooms: Create rooms and corridors with a connectivity graph
void do_rooms()
{
  int i, room_index;
  struct Room *room;
  AGENT *monster;
  int left_out;
  Coord top;
  Coord bsze;
  Coord mp;
  int old_lev;
  int endline;

  endline = maxrow+1;
  old_lev = level;
  //bsze is the maximum room size
  bsze.x = COLS/3;
  bsze.y = endline/3;
  //Clear things for a new level
  for (room = rooms; room<&rooms[MAXROOMS]; room++) room->goldval = room->num_exits = room->flags = 0;
  //Put the gone rooms, if any, on the level
  left_out = rnd(4);
  for (i = 0; i<left_out; i++)
  {
    do { 
      room = &rooms[(room_index = rnd_room())];
    } while (room->flags&ISMAZE);
    room->flags |= ISGONE;
    if (room_index>2 && level>10 && rnd(20)<level-9)
      room->flags |= ISMAZE;
  }
  //dig and populate all the rooms on the level
  for (i = 0, room = rooms; i<MAXROOMS; room++, i++)
  {
    //Find upper left corner of box that this room goes in
    top.x = (i%3)*bsze.x+1;
    top.y = i/3*bsze.y;
    if (room->flags&ISGONE)
    {
      //If the gone room is a maze room, draw the maze and set the size equal to the maximum possible.
      if (room->flags&ISMAZE) {room->pos.x = top.x; room->pos.y = top.y; draw_maze(room);}
      else
      {
        //Place a gone room.  Make certain that there is a blank line for passage drawing.
        do
        {
          room->pos.x = top.x+rnd(bsze.x-2)+1;
          room->pos.y = top.y+rnd(bsze.y-2)+1;
          room->size.x = -COLS;
          room->size.x = -endline;
        } while (!(room->pos.y>0 && room->pos.y<endline-1));
      }
      continue;
    }
    if (rnd(10)<(level-1)) room->flags |= ISDARK;
    //Find a place and size for a random room
    do
    {
      room->size.x = rnd(bsze.x-4)+4;
      room->size.y = rnd(bsze.y-4)+4;
      room->pos.x = top.x+rnd(bsze.x-room->size.x);
      room->pos.y = top.y+rnd(bsze.y-room->size.y);
    } while (room->pos.y==0);
    draw_room(room);
    //Put the gold in
    if ((rnd(2)==0) && (!had_amulet() || (level>=max_level)))
    {
      ITEM *gold;

      if ((gold = create_item(GOLD, 0))!=NULL)
      {
        gold->gold_value = room->goldval = GOLDCALC;
        while (1)
        {
          byte gch;

          rnd_pos(room, &room->gold);
          gch = get_tile(room->gold.y, room->gold.x);
          if (isfloor(gch)) break;
        }
        gold->pos = room->gold;
        gold->flags = ISMANY;
        gold->group = GOLDGRP;
        attach_item(&lvl_obj, gold);
        set_tile(room->gold.y, room->gold.x, GOLD);
      }
    }
    //Put the monster in
    if (rnd(100)<(room->goldval>0?80:25))
    {
      if ((monster = create_agent())!=NULL)
      {
        byte mch;

        do {
          rnd_pos(room, &mp); 
          mch = display_character(mp.y, mp.x);
        } while (!isfloor(mch));
        new_monster(monster, randmonster(FALSE), &mp);
        give_pack(monster);
      }
    }
  }
}

//draw_room: Draw a box around a room and lay down the floor
void draw_room(struct Room *rp)
{
  int y, x;

  //Here we draw normal rooms, one side at a time
  vert(rp, rp->pos.x); //Draw left side
  vert(rp, rp->pos.x+rp->size.x-1); //Draw right side
  horiz(rp, rp->pos.y); //Draw top
  horiz(rp, rp->pos.y+rp->size.y-1); //Draw bottom
  set_tile(rp->pos.y, rp->pos.x, ULWALL);
  set_tile(rp->pos.y, rp->pos.x+rp->size.x-1, URWALL);
  set_tile(rp->pos.y+rp->size.y-1, rp->pos.x, LLWALL);
  set_tile(rp->pos.y+rp->size.y-1, rp->pos.x+rp->size.x-1, LRWALL);
  //Put the floor down
  for (y = rp->pos.y+1; y<rp->pos.y+rp->size.y-1; y++)
    for (x = rp->pos.x+1; x<rp->pos.x+rp->size.x-1; x++)
      set_tile(y, x, FLOOR);
}

//vert: Draw a vertical line
void vert(struct Room *rp, int startx)
{
  int y;

  for (y = rp->pos.y+1; y<=rp->size.y+rp->pos.y-1; y++)
    set_tile(y, startx, VWALL);
}

//horiz: Draw a horizontal line
void horiz(struct Room *rp, int starty)
{
  int x;

  for (x = rp->pos.x; x<=rp->pos.x+rp->size.x-1; x++) 
    set_tile(starty, x, HWALL);
}

//rnd_pos: Pick a random spot in a room
void rnd_pos(struct Room *rp, Coord *cp)
{
  cp->x = rp->pos.x+rnd(rp->size.x-2)+1;
  cp->y = rp->pos.y+rnd(rp->size.y-2)+1;
}

//enter_room: Code that is executed whenever you appear in a room
void enter_room(Coord *cp)
{
  struct Room *room;
  int y, x;
  AGENT *monster;

  room = player.room = roomin(cp);
  if (bailout || (room->flags&ISGONE && (room->flags&ISMAZE)==0))
  {
    debug("in a gone room");
    return;
  }
  door_open(room);
  if (!(room->flags&ISDARK) && !on(player, ISBLIND) && !(room->flags&ISMAZE))
    for (y = room->pos.y; y<room->size.y+room->pos.y; y++)
    {
      move(y, room->pos.x);
      for (x = room->pos.x; x<room->size.x+room->pos.x; x++)
      {
        //Displaying monsters is all handled in the chase code now
        monster = monster_at(y, x);
        if (monster==NULL || !can_see_monst(monster)) 
          addch(get_tile(y, x));
        else {
          monster->oldch = get_tile(y,x); 
          addch(monster->disguise);
        }
      }
    }
}

//leave_room: Code for when we exit a room
void leave_room(Coord *cp)
{
  int y, x;
  struct Room *rp;
  byte floor;
  byte ch;

  rp = player.room;
  player.room = &passages[get_flags(cp->y, cp->x)&F_PNUM];
  floor = ((rp->flags&ISDARK) && !on(player, ISBLIND))?' ':FLOOR;
  if (rp->flags&ISMAZE) floor = PASSAGE;
  for (y = rp->pos.y+1; y<rp->size.y+rp->pos.y-1; y++) {
    for (x = rp->pos.x+1; x<rp->size.x+rp->pos.x-1; x++) {
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
          if (on(player, SEEMONST)) {
            standout(); 
            addch(ch); 
            standend(); 
            break;
          }
          else 
            monster_at(y, x)->oldch = '@';
          addch(floor);
      }
    }
  }
  door_open(rp);
}
