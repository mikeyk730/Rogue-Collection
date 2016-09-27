//Draw the connecting passages
//passages.c  1.4 (A.I. Design)       12/14/84

#include <stdlib.h>

#include "rogue.h"
#include "room.h"
#include "maze.h"
#include "main.h"
#include "output_interface.h"
#include "misc.h"
#include "passages.h"
#include "io.h"
#include "level.h"
#include "game_state.h"

extern Room rooms[];
extern Room passages[];

static int pnum;
static byte newpnum;

Coord north(Coord p)
{
    return{ p.x, p.y - 1 };
}

Coord south(Coord p)
{
    return{ p.x, p.y + 1 };
}

Coord east(Coord p)
{
    return{ p.x + 1, p.y };
}

Coord west(Coord p)
{
    return{ p.x - 1, p.y };
}

//conn: Draw a corridor from a room in a certain direction.
void conn(int r1, int r2)
{
  struct Room *room_from, *room_to;
  int rmt, rm;
  int distance, turn_spot, turn_distance;
  int direc;
  Coord del, curr, turn_delta, spos, epos;

  if (r1<r2)
  {
    rm = r1;
    if (r1+1==r2) direc = 'r'; else direc = 'd';
  }
  else
  {
    rm = r2;
    if (r2+1==r1) direc = 'r'; else direc = 'd';
  }
  room_from = &rooms[rm];
  //Set up the movement variables, in two cases: first drawing one down.
  if (direc=='d')
  {
    rmt = rm+3; //room # of dest
    room_to = &rooms[rmt]; //room pointer of dest
    del.x = 0; //direction of move
    del.y = 1;
    //If we are drawing from/to regular or maze rooms, we have to pick the spot we draw from/to
    if ((room_from->is_gone())==0 || (room_from->is_maze()))
    {
      spos.y = room_from->pos.y+room_from->size.y-1;
      do {
          spos.x = room_from->pos.x + rnd(room_from->size.x - 2) + 1;
      } while (game->level().get_tile(spos)==' ');
    }
    else {spos.x = room_from->pos.x; spos.y = room_from->pos.y;}
    epos.y = room_to->pos.y;
    if ((room_to->is_gone())==0 || (room_to->is_maze()))
    {
      do {
          epos.x = room_to->pos.x + rnd(room_to->size.x - 2) + 1;
      } while (game->level().get_tile(epos)==' ');
    }
    else epos.x = room_to->pos.x;
    distance = abs(spos.y-epos.y)-1; //distance to move
    turn_delta.y = 0; //direction to turn
    turn_delta.x = (spos.x<epos.x?1:-1);
    turn_distance = abs(spos.x-epos.x); //how far to turn
  }
  else if (direc=='r')
  {
    //setup for moving right
    rmt = rm+1;
    room_to = &rooms[rmt];
    del.x = 1;
    del.y = 0;
    if ((room_from->is_gone())==0 || (room_from->is_maze()))
    {
      spos.x = room_from->pos.x+room_from->size.x-1;
      do {
          spos.y = room_from->pos.y+rnd(room_from->size.y-2)+1;
      } while (game->level().get_tile(spos)==' ');
    }
    else {
        spos.x = room_from->pos.x; 
        spos.y = room_from->pos.y;
    }
    epos.x = room_to->pos.x;
    if ((room_to->is_gone())==0 || (room_to->is_maze()))
    {
      do {
          epos.y = room_to->pos.y+rnd(room_to->size.y-2)+1;
      } while (game->level().get_tile(epos)==' ');
    }
    else epos.y = room_to->pos.y;
    distance = abs(spos.x-epos.x)-1;
    turn_delta.y = (spos.y<epos.y?1:-1);
    turn_delta.x = 0;
    turn_distance = abs(spos.y-epos.y);
  }
  else debug("error in connection tables");

  turn_spot = rnd(distance-1)+1;
  //Draw in the doors on either side of the passage or just put #'s if the rooms are gone.
  if (!(room_from->is_gone())) 
      add_door(room_from, &spos);
  else
      psplat(spos);
  if (!(room_to->is_gone())) 
      add_door(room_to, &epos);
  else 
      psplat(epos);
  //Get ready to move...
  curr.x = spos.x;
  curr.y = spos.y;
  while (distance)
  {
    //Move to new position
    curr.x += del.x;
    curr.y += del.y;
    //Check if we are at the turn place, if so do the turn
    if (distance==turn_spot)
    {
      while (turn_distance--) {
          psplat(curr); 
          curr.x += turn_delta.x; 
          curr.y += turn_delta.y;
      }
    }
    //Continue digging along
    psplat(curr);
    distance--;
  }
  curr.x += del.x;
  curr.y += del.y;
  if (!equal(curr, epos)) {
      epos.x -= del.x;
      epos.y -= del.y;
      psplat(epos);
  }
}

//do_passages: Draw all the passages on a level.
void do_passages()
{
  int i, j;
  int roomcount;
  static struct rdes
  {
    char conn[MAXROOMS]; //possible to connect to room i?
    char isconn[MAXROOMS]; //connection been made to room i?
    char ingraph; //this room in graph already?
  } rdes[MAXROOMS] =
  {
    { {0, 1, 0, 1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {1, 0, 1, 0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {0, 1, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {1, 0, 0, 0, 1, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {0, 1, 0, 1, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {0, 0, 1, 0, 1, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {0, 0, 0, 1, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {0, 0, 0, 0, 1, 0, 1, 0, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0},
    { {0, 0, 0, 0, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}, 0}
  };
  struct rdes *r1, *r2;

  //reinitialize room graph description
  for (r1 = rdes; r1<&rdes[MAXROOMS]; r1++)
  {
    for (j = 0; j<MAXROOMS; j++) r1->isconn[j] = false;
    r1->ingraph = false;
  }
  //starting with one room, connect it to a random adjacent room and then pick a new room to start with.
  roomcount = 1;
  r1 = &rdes[rnd(MAXROOMS)];
  r1->ingraph = true;
  do
  {
    //find a room to connect with
    j = 0;
    for (i = 0; i<MAXROOMS; i++) if (r1->conn[i] && !rdes[i].ingraph && rnd(++j)==0) r2 = &rdes[i];
    //if no adjacent rooms are outside the graph, pick a new room to look from
    if (j==0)
    {
      do r1 = &rdes[rnd(MAXROOMS)]; while (!r1->ingraph);
    }
    //otherwise, connect new room to the graph, and draw a tunnel to it
    else
    {
      r2->ingraph = true;
      i = r1-rdes;
      j = r2-rdes;
      conn(i, j);
      r1->isconn[j] = true;
      r2->isconn[i] = true;
      roomcount++;
    }
  } while (roomcount<MAXROOMS);
  //attempt to add passages to the graph a random number of times so that there isn't always just one unique passage through it.
  for (roomcount = rnd(5); roomcount>0; roomcount--)
  {
    r1 = &rdes[rnd(MAXROOMS)]; //a random room to look from
    //find an adjacent room not already connected
    j = 0;
    for (i = 0; i<MAXROOMS; i++) if (r1->conn[i] && !r1->isconn[i] && rnd(++j)==0) r2 = &rdes[i];
    //if there is one, connect it and look for the next added passage
    if (j!=0)
    {
      i = r1-rdes;
      j = r2-rdes;
      conn(i, j);
      r1->isconn[j] = true;
      r2->isconn[i] = true;
    }
  }
  passnum();
}

//door: Add a door or possibly a secret door.  Also enters the door in the exits array of the room.
void add_door(struct Room *rm, Coord *cp)
{
    int xit;

    if (rnd(10) + 1 < get_level() && rnd(5) == 0 && !game->wizard().no_hidden_doors())
    {
        game->level().set_tile(*cp, (cp->y == rm->pos.y || cp->y == rm->pos.y + rm->size.y - 1) ? HWALL : VWALL);
        game->level().unset_flag(*cp, F_REAL);
    }
    else 
        game->level().set_tile(*cp, DOOR);
    xit = rm->num_exits++;
    rm->exits[xit].y = cp->y;
    rm->exits[xit].x = cp->x;
}

//add_pass: Add the passages to the current window (wizard command)
void add_pass()
{
    int y, x, ch;
    const int COLS = game->screen().columns();

    for (y = 1; y < maxrow; y++)
        for (x = 0; x < COLS; x++)
            if ((ch = game->level().get_tile({x, y})) == DOOR || ch == PASSAGE)
                game->screen().mvaddch({ x, y }, ch);
}

//passnum: Assign a number to each passageway
void passnum()
{
  struct Room *room;
  int i;

  pnum = 0;
  newpnum = false;
  for (room = passages; room<&passages[MAXPASS]; room++) room->num_exits = 0;
  for (room = rooms; room<&rooms[MAXROOMS]; room++) for (i = 0; i<room->num_exits; i++)
  {
    newpnum++;
    numpass(room->exits[i]);
  }
}

//numpass: Number a passageway square and its brethren
void numpass(Coord p)
{
  struct Room *room;
  byte ch;

  if (offmap(p)) return;
  if (game->level().get_passage_num(p))
      return;
  if (newpnum) {pnum++; newpnum = false;}
  //check to see if it is a door or secret door, i.e., a new exit, or a numberable type of place
  if ((ch = game->level().get_tile(p))==DOOR || (!game->level().is_real(p) && ch!=FLOOR))
  {
    room = &passages[pnum];
    room->exits[room->num_exits].y = p.y;
    room->exits[room->num_exits++].x = p.x;
  }
  else if (!game->level().is_passage(p))
      return;
  game->level().set_flag(p, pnum);
  //recurse on the surrounding places
  numpass(south(p));
  numpass(north(p));
  numpass(east(p));
  numpass(west(p));
}

void psplat(Coord p)
{
    game->level().set_tile(p, PASSAGE);
    game->level().set_flag(p, F_PASS);
}
