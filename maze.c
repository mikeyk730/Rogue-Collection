//Maze drawing routines.  Based on the algorithm presented in the December 1981 Byte "How to Build a Maze" by David Matuszek.
//maze.c      1.4     (A.I. Design)   12/14/84

#include "rogue.h"
#include "maze.h"
#include "main.h"
#include "misc.h"
#include "rooms.h"
#include "io.h"

#define MAXFRNT  100
#define FRONTIER  'F'
#define NOTHING  ' '
#define MAXY  (topy+((maxrow+1)/3))
#define MAXX  (topx+COLS/3)

extern int maxrow;

static int frcnt, ny, nx, topy, topx;
static int maxx, maxy;
static int *fr_y, *fr_x;

void draw_maze(struct room *rp)
{
  int y, x;
  int fy[MAXFRNT], fx[MAXFRNT];
  int psgcnt;
  coord spos;

  fr_y = fy;
  fr_x = fx;
  maxx = maxy = 0;
  topy = rp->r_pos.y;
  if (topy==0) topy = ++rp->r_pos.y;
  topx = rp->r_pos.x;
  //Choose a random spot in the maze and initialize the frontier to be the immediate neighbors of this random spot.
  y = topy;
  x = topx;
  splat(y, x);
  new_frontier(y, x);
  //While there are new frontiers, connect them to the path and possibly expand the frontier even more.
  while (frcnt) {con_frnt(); new_frontier(ny, nx);}
  //According to the Grand Beeking, every maze should have a loop. Don't worry if you don't understand this.
  rp->r_max.x = maxx-rp->r_pos.x+1;
  rp->r_max.y = maxy-rp->r_pos.y+1;
  do
  {
    static coord ld[4] = {-1, 0, 0, 1, 1, 0, 0, -1};
    coord *cp;
    int sh;

    rnd_pos(rp, &spos);
    for (psgcnt = 0, cp = ld, sh = 1; cp<&ld[4]; sh <<= 1, cp++)
    {
      y = cp->y+spos.y; x = cp->x+spos.x;
      if (!offmap(y, x) && chat(y, x)==PASSAGE) psgcnt += sh;
    }
  } while (chat(spos.y, spos.x)==PASSAGE || psgcnt%5);
  splat(spos.y, spos.x);
}

void new_frontier(int y, int x)
{
  add_frnt(y-2, x);
  add_frnt(y+2, x);
  add_frnt(y, x-2);
  add_frnt(y, x+2);
}

void add_frnt(int y, int x)
{
#ifdef DEBUG
  if (frcnt==MAXFRNT-1) debug("MAZE DRAWING ERROR #3\n");
#endif
  if (inrange(y, x) && chat(y, x)==NOTHING)
  {
    chat(y, x) = FRONTIER;
    fr_y[frcnt] = y;
    fr_x[frcnt++] = x;
  }
}

//Connect randomly to one of the adjacent points in the spanning tree
void con_frnt()
{
  int n, which, ydelt = 0, xdelt = 0;
  int choice[4];
  int cnt = 0, y, x;

  //Choose a random frontier
  n = rnd(frcnt);
  ny = fr_y[n];
  nx = fr_x[n];
  fr_y[n] = fr_y[frcnt-1];
  fr_x[n] = fr_x[--frcnt];
  //Count and collect the adjacent points we can connect to
  if (maze_at(ny-2, nx)>0) choice[cnt++] = 0;
  if (maze_at(ny+2, nx)>0) choice[cnt++] = 1;
  if (maze_at(ny, nx-2)>0) choice[cnt++] = 2;
  if (maze_at(ny, nx+2)>0) choice[cnt++] = 3;
  //Choose one of the open places, connect to it and then the task is complete
  which = choice[rnd(cnt)];
  splat(ny, nx);
  switch (which)
  {
    case 0: which = 1; ydelt = -1; break;
    case 1: which = 0; ydelt = 1; break;
    case 2: which = 3; xdelt = -1; break;
    case 3: which = 2; xdelt = 1; break;
  }
  y = ny+ydelt;
  x = nx+xdelt;
  if (inrange(y, x)) splat(y, x);
}

int maze_at(int y, int x)
{
  if (inrange(y, x) && chat(y, x)==PASSAGE) return 1;
  else return 0;
}

void splat(int y, int x)
{
  chat(y, x) = PASSAGE;
  flat(y, x) = F_MAZE|F_REAL;
  if (x>maxx) maxx = x;
  if (y>maxy) maxy = y;
}

int inrange(int y, int x)
{
  return (y>=topy && y<MAXY && x>=topx && x<MAXX);
}
