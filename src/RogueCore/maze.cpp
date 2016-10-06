//Maze drawing routines.  Based on the algorithm presented in the December 1981 Byte "How to Build a Maze" by David Matuszek.
//maze.c      1.4     (A.I. Design)   12/14/84

#include "rogue.h"
#include "maze.h"
#include "main.h"
#include "misc.h"
#include "rooms.h"
#include "io.h"
#include "level.h"
#include "room.h"
#include "game_state.h"
#include "curses.h"

#define MAXFRNT  100
#define FRONTIER  'F'
#define NOTHING  ' '
#define MAXY  (topy+((maxrow()+1)/3))
#define MAXX  (topx+COLS/3)

static int frcnt, ny, nx, topy, topx;
static int maxx, maxy;
static int *fr_y, *fr_x;

void draw_maze(struct Room *room)
{
    int y, x;
    int fy[MAXFRNT], fx[MAXFRNT];
    int psgcnt;
    Coord spos;

    fr_y = fy;
    fr_x = fx;
    maxx = maxy = 0;
    topy = room->m_ul_corner.y;
    if (topy == 0) topy = ++room->m_ul_corner.y;
    topx = room->m_ul_corner.x;
    //Choose a random spot in the maze and initialize the frontier to be the immediate neighbors of this random spot.
    y = topy;
    x = topx;
    splat({ x,y });
    new_frontier({ x,y });
    //While there are new frontiers, connect them to the path and possibly expand the frontier even more.
    while (frcnt) {
        con_frnt();
        new_frontier({ nx,ny });
    }
    //According to the Grand Beeking, every maze should have a loop. Don't worry if you don't understand this.
    room->m_size.x = maxx - room->m_ul_corner.x + 1;
    room->m_size.y = maxy - room->m_ul_corner.y + 1;
    do
    {
        static Coord ld[4] = { -1, 0, 0, 1, 1, 0, 0, -1 };
        Coord *cp;
        int sh;

        rnd_pos(room, &spos);
        for (psgcnt = 0, cp = ld, sh = 1; cp < &ld[4]; sh <<= 1, cp++)
        {
            y = cp->y + spos.y; x = cp->x + spos.x;
            if (!offmap({ x,y }) && game->level().get_tile({ x, y }) == PASSAGE) psgcnt += sh;
        }
    } while (game->level().get_tile(spos) == PASSAGE || psgcnt % 5);
    splat(spos);
}

void new_frontier(Coord p)
{
    add_frnt({ p.x,     p.y - 2 });
    add_frnt({ p.x,     p.y + 2 });
    add_frnt({ p.x - 2, p.y });
    add_frnt({ p.x + 2, p.y });
}

void add_frnt(Coord p)
{
    if (frcnt == MAXFRNT - 1) debug("MAZE DRAWING ERROR #3\n");

    if (inrange(p) && game->level().get_tile(p) == NOTHING)
    {
        game->level().set_tile(p, FRONTIER);
        fr_y[frcnt] = p.y;
        fr_x[frcnt++] = p.x;
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
    fr_y[n] = fr_y[frcnt - 1];
    fr_x[n] = fr_x[--frcnt];
    //Count and collect the adjacent points we can connect to
    if (maze_at({ nx, ny - 2 }) > 0) choice[cnt++] = 0;
    if (maze_at({ nx, ny + 2 }) > 0) choice[cnt++] = 1;
    if (maze_at({ nx - 2, ny }) > 0) choice[cnt++] = 2;
    if (maze_at({ nx + 2, ny }) > 0) choice[cnt++] = 3;
    //Choose one of the open places, connect to it and then the task is complete
    which = choice[rnd(cnt)];
    splat({ nx,ny });
    switch (which)
    {
    case 0: which = 1; ydelt = -1; break;
    case 1: which = 0; ydelt = 1; break;
    case 2: which = 3; xdelt = -1; break;
    case 3: which = 2; xdelt = 1; break;
    }
    y = ny + ydelt;
    x = nx + xdelt;
    if (inrange({ x,y })) splat({ x,y });
}

int maze_at(Coord p)
{
    if (inrange(p) && game->level().get_tile(p) == PASSAGE) return 1;
    else return 0;
}

void splat(Coord p)
{
    game->level().set_tile(p, PASSAGE);
    game->level().copy_flags(p, F_MAZE | F_REAL);
    if (p.x > maxx)
        maxx = p.x;
    if (p.y > maxy)
        maxy = p.y;
}

int inrange(Coord p)
{
    const int COLS = game->screen().columns();
    return (p.y >= topy && p.y < MAXY && p.x >= topx && p.x < MAXX);
}
