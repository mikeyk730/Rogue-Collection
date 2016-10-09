/*
 * Create the layout for the new level
 *
 * @(#)rooms.c	4.16 (Berkeley) 1/12/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <ctype.h>
#include <curses.h>
#include "rogue.h"

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
    coord bsze;
    coord mp;

    /*
     * bsze is the maximum room size
     */
    bsze.x = COLS/3;
    bsze.y = LINES/3;
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
	top.x = (i%3)*bsze.x + 1;
	top.y = i/3*bsze.y;
	if (rp->r_flags & ISGONE)
	{
	    /*
	     * Place a gone room.  Make certain that there is a blank line
	     * for passage drawing.
	     */
	    do
	    {
		rp->r_pos.x = top.x + rnd(bsze.x-2) + 1;
		rp->r_pos.y = top.y + rnd(bsze.y-2) + 1;
		rp->r_max.x = -COLS;
		rp->r_max.x = -LINES;
	    } until (rp->r_pos.y > 0 && rp->r_pos.y < LINES-1);
	    continue;
	}
	if (rnd(10) < level - 1)
	    rp->r_flags |= ISDARK;
	/*
	 * Find a place and size for a random room
	 */
	do
	{
	    rp->r_max.x = rnd(bsze.x - 4) + 4;
	    rp->r_max.y = rnd(bsze.y - 4) + 4;
	    rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
	    rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
	} until (rp->r_pos.y != 0);
	/*
	 * Put the gold in
	 */
	if (rnd(2) == 0 && (!amulet || level >= max_level))
	{
	    register THING *gold;

	    gold = new_item();
	    gold->o_goldval = rp->r_goldval = GOLDCALC;
	    rnd_pos(rp, &rp->r_gold);
	    gold->o_pos = rp->r_gold;
	    gold->o_flags = ISMANY;
	    gold->o_group = GOLDGRP;
	    gold->o_type = GOLD;
	    attach(lvl_obj, gold);
	}
	draw_room(rp);
	/*
	 * Put the monster in
	 */
	if (rnd(100) < (rp->r_goldval > 0 ? 80 : 25))
	{
	    tp = new_item();
	    do
	    {
		rnd_pos(rp, &mp);
	    } until (winat(mp.y, mp.x) == FLOOR);
	    new_monster(tp, randmonster(FALSE), &mp);
	    give_pack(tp);
	}
    }
}

/*
 * draw_room:
 *	Draw a box around a room and lay down the floor
 */
draw_room(rp)
register struct room *rp;
{
    register int y, x;

    vert(rp, rp->r_pos.x);				/* Draw left side */
    vert(rp, rp->r_pos.x + rp->r_max.x - 1);		/* Draw right side */
    horiz(rp, rp->r_pos.y);				/* Draw top */
    horiz(rp, rp->r_pos.y + rp->r_max.y - 1);		/* Draw bottom */
    /*
     * Put the floor down
     */
    for (y = rp->r_pos.y + 1; y < rp->r_pos.y + rp->r_max.y - 1; y++)
	/*strrep(&chat(rp->r_pos.y + 1, j), FLOOR, rp->r_max.y - rp->r_pos.y - 2);*/
	for (x = rp->r_pos.x + 1; x < rp->r_pos.x + rp->r_max.x - 1; x++)
	    chat(y, x) = FLOOR;
    /*
     * Put the gold there
     */
    if (rp->r_goldval)
	chat(rp->r_gold.y, rp->r_gold.x) = GOLD;
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
	chat(y, startx) = '|';
}

/*
 * horiz:
 *	Draw a horizontal line
 */
horiz(rp, starty)
register struct room *rp;
int starty;
{
    register int x;

    for (x = rp->r_pos.x; x <= rp->r_pos.x + rp->r_max.x - 1; x++)
	chat(starty, x) = '-';
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
    if (rp->r_flags & ISGONE)
    {
	msg("in a gone room");
	return;
    }
    door_open(rp);
    if (!(rp->r_flags & ISDARK) && !on(player, ISBLIND))
	for (y = rp->r_pos.y; y < rp->r_max.y + rp->r_pos.y; y++)
	{
	    move(y, rp->r_pos.x);
	    for (x = rp->r_pos.x; x < rp->r_max.x + rp->r_pos.x; x++)
	    {
		tp = moat(y, x);
		if (tp == NULL || !see_monst(tp))
		    addch(chat(y, x));
		else
		    addch(tp->t_disguise);
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
    register char floor;
    register char ch;

    rp = proom;
    proom = &passages[flat(cp->y, cp->x) & F_PNUM];
    floor = ((rp->r_flags & ISDARK) && !on(player, ISBLIND)) ? ' ' : FLOOR;
    for (y = rp->r_pos.y + 1; y < rp->r_max.y + rp->r_pos.y - 1; y++)
	for (x = rp->r_pos.x + 1; x < rp->r_max.x + rp->r_pos.x - 1; x++)
	    switch (ch = mvinch(y, x))
	    {
		case ' ':
		case TRAP:
		case STAIRS:
		    break;
		case FLOOR:
		    if (floor == ' ')
			addch(' ');
		    break;
		default:
		    /*
		     * to check for monster, we have to strip out
		     * standout bit
		     */
		    if (isupper(toascii(ch)))
			if (on(player, SEEMONST))
			{
			    standout();
			    addch(ch);
			    standend();
			    break;
			}
			else
                        {
                            THING *tp = moat(y, x);

                            if (tp != NULL)
                                tp->t_oldch = floor;

#ifdef WIZARD
                            else
                                msg("couldn't find monster in leave_room at (%d,%d)", y, x);
#endif
                        }

		    addch(floor);
	    }
    door_open(rp);
}
