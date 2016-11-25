/*
 * Create the layout for the new level
 *
 * @(#)rooms.c	4.21 (NMT from Berkeley 5.2) 8/25/83
 */

#include <ctype.h>
#include <curses.h>
#include "rogue.h"

typedef struct spot {		/* position matrix for maze positions */
	int	nexits;
	coord	exits[20];
	int	used;
} SPOT;

#define GOLDGRP 1

/*
 * do_rooms:
 *	Create rooms and corridors with a connectivity graph
 */
do_rooms()
{
    register int i;
    register struct room *rp;
    register THING *tp;
    register int left_out;
    coord top;
    coord bsze;				/* maximum room size */
    coord mp;

    bsze.x = COLS / 3;
    bsze.y = LINES / 3;
    /*
     * Clear things for a new level
     */
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	rp->r_goldval = rp->r_nexits = rp->r_flags = 0;
    /*
     * Put the gone rooms, if any, on the level
     */
    left_out = rnd(4);
    for (i = 0; i < left_out; i++)
	rooms[rnd_room()].r_flags |= ISGONE;
    /*
     * dig and populate all the rooms on the level
     */
    for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++)
    {
	/*
	 * Find upper left corner of box that this room goes in
	 */
	top.x = (i % 3) * bsze.x + 1;
	top.y = (i / 3) * bsze.y;
	if (rp->r_flags & ISGONE)
	{
	    /*
	     * Place a gone room.  Make certain that there is a blank line
	     * for passage drawing.
	     */
	    do
	    {
		rp->r_pos.x = top.x + rnd(bsze.x - 2) + 1;
		rp->r_pos.y = top.y + rnd(bsze.y - 2) + 1;
		rp->r_max.x = -COLS;
		rp->r_max.y = -LINES;
	    } until (rp->r_pos.y > 0 && rp->r_pos.y < LINES-1);
	    continue;
	}
	/*
	 * set room type
	 */
	if (rnd(10) < level - 1)
	{
	    rp->r_flags |= ISDARK;		/* dark room */
	    if (rnd(15) == 0)
		rp->r_flags = ISMAZE;		/* maze room */
	}
	/*
	 * Find a place and size for a random room
	 */
	if (rp->r_flags & ISMAZE)
	{
	    rp->r_max.x = bsze.x - 1;
	    rp->r_max.y = bsze.y - 1;
	    if ((rp->r_pos.x = top.x) == 1)
		rp->r_pos.x = 0;
	    if ((rp->r_pos.y = top.y) == 0)
	    {
		rp->r_pos.y++;
		rp->r_max.y--;
	    }
	}
	else
	    do
	    {
		rp->r_max.x = rnd(bsze.x - 4) + 4;
		rp->r_max.y = rnd(bsze.y - 4) + 4;
		rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
		rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
	    } until (rp->r_pos.y != 0);
	draw_room(rp);
	/*
	 * Put the gold in
	 */
	if (rnd(2) == 0 && (!amulet || level >= max_level))
	{
	    register THING *gold;

	    gold = new_item();
	    gold->o_goldval = rp->r_goldval = GOLDCALC;
	    find_floor(rp, &rp->r_gold, FALSE, FALSE);
	    gold->o_pos = rp->r_gold;
	    chat(rp->r_gold.y, rp->r_gold.x) = GOLD;
	    gold->o_flags = ISMANY;
	    gold->o_group = GOLDGRP;
	    gold->o_type = GOLD;
	    attach(lvl_obj, gold);
	}
	/*
	 * Put the monster in
	 */
	if (rnd(100) < (rp->r_goldval > 0 ? 80 : 25))
	{
	    tp = new_item();
	    find_floor(rp, &mp, FALSE, TRUE);
	    new_monster(tp, randmonster(FALSE), &mp);
	    give_pack(tp);
	}
    }
}

/*
 * draw_room:
 *	Draw a box around a room and lay down the floor for normal
 *	rooms; for maze rooms, draw maze.
 */
draw_room(rp)
register struct room *rp;
{
    register int y, x;

    if (rp->r_flags & ISMAZE)
	do_maze(rp);
    else
    {
	vert(rp, rp->r_pos.x);				/* Draw left side */
	vert(rp, rp->r_pos.x + rp->r_max.x - 1);	/* Draw right side */
	horiz(rp, rp->r_pos.y, 1);				/* Draw top */
	horiz(rp, rp->r_pos.y + rp->r_max.y - 1, 0);	/* Draw bottom */

	/*
	 * Put the floor down
	 */
	for (y = rp->r_pos.y + 1; y < rp->r_pos.y + rp->r_max.y - 1; y++)
	    for (x = rp->r_pos.x + 1; x < rp->r_pos.x + rp->r_max.x - 1; x++)
		chat(y, x) = FLOOR;
    }
}

/*
 * vert:
 *	Draw a vertical line
 */
vert(rp, startx)
register struct room *rp;
register int startx;
{
    register int y;

    for (y = rp->r_pos.y + 1; y <= rp->r_max.y + rp->r_pos.y - 1; y++)
	chat(y, startx) = VWALL;
}

/*
 * horiz:
 *	Draw a horizontal line
 */
horiz(rp, starty, is_top)
register struct room *rp;
int starty;
int is_top;
{
    register int x;

    chat(starty, rp->r_pos.x) = is_top ? ULWALL : LLWALL;
    for (x = rp->r_pos.x+1; x < rp->r_pos.x + rp->r_max.x - 1; x++)
	chat(starty, x) = HWALL;
    chat(starty, rp->r_pos.x + rp->r_max.x - 1) = is_top ? URWALL : LRWALL;
}

/*
 * do_maze:
 *	Dig a maze
 */

static int	Maxy, Maxx, Starty, Startx;

static SPOT	Maze[MAXCOLS/3+1][MAXLINES/3+1];

do_maze(rp)
register struct room *rp;
{
	register int	starty, startx;

	for (starty = 0; starty < MAXCOLS/3; starty++)
		for (startx = 0; startx < MAXLINES/3; startx++)
			Maze[starty][startx].used = Maze[starty][startx].nexits = 0;

	Maxy = rp->r_max.y;
	Maxx = rp->r_max.x;
	Starty = rp->r_pos.y;
	Startx = rp->r_pos.x;
	starty = (rnd(rp->r_max.y) / 2) * 2;
	startx = (rnd(rp->r_max.x) / 2) * 2;
	chat(starty + Starty, startx + Startx) = PASSAGE;
	flat(starty + Starty, startx + Startx) |= F_PASS;
	dig(starty, startx);
}

/*
 * dig:
 *	Dig out from around where we are now, if possible
 */
dig(y, x)
register int y, x; {

    register int i, count, newy, newx, nexty, nextx;
    register SPOT *sp;
    static int dely[] = { 2,-2, 0, 0 };
    static int delx[] = { 0, 0, 2,-2 };

    for (;;)
    {
	count = 0;
	for (i = 0; i < 4; i++)
	{
	    newy = y + dely[i];
	    newx = x + delx[i];
	    if (newy < 0 || newy > Maxy || newx < 0 || newx > Maxx)
		continue;
	    if (chat(newy + Starty, newx + Startx) != ' ')
		continue;
	    if (rnd(++count) == 0)
	    {
		nexty = newy;
		nextx = newx;
	    }
	}
	if (count == 0)
	    return;
	sp = &Maze[y][x];
	sp->exits[sp->nexits].y = nexty;
	sp->exits[sp->nexits++].x = nextx;
	sp = &Maze[nexty][nextx];
	sp->exits[sp->nexits].y = y;
	sp->exits[sp->nexits++].x = x;
	if (nexty == y)
	    if (nextx - x < 0)
	    {
		i = INDEX(nexty + Starty, nextx + Startx + 1);
		_level[i] = PASSAGE;
		_flags[i] |= F_PASS;
	    }
	    else
	    {
		i = INDEX(nexty + Starty, nextx + Startx - 1);
		_level[i] = PASSAGE;
		_flags[i] |= F_PASS;
	    }
	else
	    if (nexty - y < 0)
	    {
		i = INDEX(nexty + Starty + 1, nextx + Startx);
		_level[i] = PASSAGE;
		_flags[i] |= F_PASS;
	    }
	    else
	    {
		i = INDEX(nexty + Starty - 1, nextx + Startx);
		_level[i] = PASSAGE;
		_flags[i] |= F_PASS;
	    }
	i = INDEX(nexty + Starty, nextx + Startx);
	_level[i] = PASSAGE;
	_flags[i] |= F_PASS;
	dig(nexty, nextx);
    }
}

/*
 * rnd_pos:
 *	Pick a random spot in a room
 */
rnd_pos(rp, cp)
register struct room *rp;
register coord *cp;
{
    cp->x = rp->r_pos.x + rnd(rp->r_max.x - 2) + 1;
    cp->y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
}

/*
 * find_floor:
 *	Find a valid floor spot in this room.  If rp is NULL, then
 *	pick a new room each time around the loop.
 */
find_floor(rp, cp, limit, monst)
register struct room *rp;
register coord *cp;
int limit;
bool monst;
{
    register int cnt;
    register unsigned char compchar;
    register bool pickroom;

    if (!(pickroom = (rp == NULL)))
	compchar = ((rp->r_flags & ISMAZE) ? PASSAGE : FLOOR);
    cnt = limit;
    for (;;)
    {
	if (limit && cnt-- == 0)
	    return FALSE;
	if (pickroom)
	{
	    rp = &rooms[rnd_room()];
	    compchar = ((rp->r_flags & ISMAZE) ? PASSAGE : FLOOR);
	}
	rnd_pos(rp, cp);
	if (monst)
	{
	    if (moat(cp->y, cp->x) == NULL && step_ok(chat(cp->y, cp->x)))
		return TRUE;
	}
	else if (chat(cp->y, cp->x) == compchar)
	    return TRUE;
    }
}

/*
 * enter_room:
 *	Code that is executed whenver you appear in a room
 */
enter_room(cp)
register coord *cp;
{
    register struct room *rp;
    register int y, x;
    register THING *tp;

    rp = proom = roomin(cp);
    door_open(rp);
    if (!(rp->r_flags & ISDARK) && !on(player, ISBLIND))
	for (y = rp->r_pos.y; y < rp->r_max.y + rp->r_pos.y; y++)
	{
	    move(y, rp->r_pos.x);
	    for (x = rp->r_pos.x; x < rp->r_max.x + rp->r_pos.x; x++)
	    {
		tp = moat(y, x);
		if (tp == NULL)
		    addrawch(chat(y, x));
		else if (!see_monst(tp))
		    if (on(player, SEEMONST))
		    {
			standout();
			addrawch(tp->t_disguise);
			standend();
		    }
		    else
			addrawch(chat(y, x));
		else
		    addrawch(tp->t_disguise);
	    }
	}
}

/*
 * leave_room:
 *	Code for when we exit a room
 */
leave_room(cp)
register coord *cp;
{
    register int y, x;
    register struct room *rp;
    register unsigned char floor;
    register unsigned char ch;

    rp = proom;
    if (rp->r_flags & ISMAZE)
	return;
    proom = &passages[flat(cp->y, cp->x) & F_PNUM];
    if ((rp->r_flags & ISDARK) && !on(player, ISBLIND))
	floor = ' ';
    else
	floor = FLOOR;
    for (y = rp->r_pos.y + 1; y < rp->r_max.y + rp->r_pos.y - 1; y++)
	for (x = rp->r_pos.x + 1; x < rp->r_max.x + rp->r_pos.x - 1; x++)
	    switch (ch = MVINCH(y, x))
	    {
		case ' ':
		case TRAP:
		    break;
		case FLOOR:
		    if (floor == ' ')
			addrawch(' ');
		    break;
		case STAIRS:
		    if (!on(player, ISTrip) ||
		       (seenstairs && stairs.y == y && stairs.x == x))
			    break;
		    /* FALLTHROUGH */
		default:
		    /*
		     * to check for monster, we have to strip out
		     * standout bit
		     */
		    if (isupper(toascii(ch)))
			if (on(player, SEEMONST))
			{
			    standout();
			    addrawch(ch);
			    standend();
			    break;
			}
			else
			    moat(y, x)->t_oldch = floor;
		    addrawch(floor);
	    }
    door_open(rp);
}
