/*
 * Routines dealing specifically with rings
 *
 * @(#)rings.c	4.13 (Berkeley) 1/28/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <string.h>
#include "rogue.h"

/*
 * ring_on:
 *	Put a ring on a hand
 */
ring_on()
{
    register THING *obj;
    register int ring;

    obj = get_item("put on", RING);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (obj == NULL)
	return;
    if (obj->o_type != RING)
    {
	if (!terse)
	    msg("it would be difficult to wrap that around a finger");
	else
	    msg("not a ring");
	return;
    }

    /*
     * find out which hand to put it on
     */
    if (is_current(obj))
	return;

    if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL)
    {
	if ((ring = gethand()) < 0)
	    return;
    }
    else if (cur_ring[LEFT] == NULL)
	ring = LEFT;
    else if (cur_ring[RIGHT] == NULL)
	ring = RIGHT;
    else
    {
	if (!terse)
	    msg("you already have a ring on each hand");
	else
	    msg("wearing two");
	return;
    }
    cur_ring[ring] = obj;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (obj->o_which)
    {
	case R_ADDSTR:
	    chg_str(obj->o_ac);
	    break;
	case R_SEEINVIS:
	    invis_on();
	    break;
	case R_AGGR:
	    aggravate();
	    break;
    }

    if (!terse)
	addmsg("you are now wearing ");
    msg("%s (%c)", inv_name(obj, TRUE), pack_char(obj));
}

/*
 * ring_off:
 *	Take off a ring
 */
ring_off()
{
    register int ring;
    register THING *obj;
    register char packchar;

    if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL)
    {
	if (terse)
	    msg("no rings");
	else
	    msg("you aren't wearing any rings");
	return;
    }
    else if (cur_ring[LEFT] == NULL)
	ring = RIGHT;
    else if (cur_ring[RIGHT] == NULL)
	ring = LEFT;
    else
	if ((ring = gethand()) < 0)
	    return;
    mpos = 0;
    obj = cur_ring[ring];
    if (obj == NULL)
    {
	msg("not wearing such a ring");
	return;
    }
    packchar = pack_char(obj);
    if (dropcheck(obj))
	msg("was wearing %s(%c)", inv_name(obj, TRUE), packchar);
}

/*
 * gethand:
 *	Which hand is the hero interested in?
 */
gethand()
{
    register int c;

    for (;;)
    {
	if (terse)
	    msg("left or right ring? ");
	else
	    msg("left hand or right hand? ");
	if ((c = readchar()) == ESCAPE)
	    return -1;
	mpos = 0;
	if (c == 'l' || c == 'L')
	    return LEFT;
	else if (c == 'r' || c == 'R')
	    return RIGHT;
	if (terse)
	    msg("L or R");
	else
	    msg("please type L or R");
    }
}

/*
 * ring_eat:
 *	How much food does this ring use up?
 */
ring_eat(hand)
register int hand;
{
    if (cur_ring[hand] == NULL)
	return 0;
    switch (cur_ring[hand]->o_which)
    {
	case R_REGEN:
	    return 2;
	case R_SUSTSTR:
	case R_SUSTARM:
	case R_PROTECT:
	case R_ADDSTR:
	case R_STEALTH:
	    return 1;
	case R_SEARCH:
	case R_ADDHIT:
	case R_ADDDAM:
	    return (rnd(3) == 0);
	case R_DIGEST:
	    return -rnd(2);
	case R_SEEINVIS:
	    return (rnd(5) == 0);
	default:
	    return 0;
    }
}

/*
 * ring_num:
 *	Print ring bonuses
 */
char *
ring_num(obj)
register THING *obj;
{
    static char buf[5];

    if (!(obj->o_flags & ISKNOW))
	return "";
    switch (obj->o_which)
    {
	case R_PROTECT:
	case R_ADDSTR:
	case R_ADDDAM:
	case R_ADDHIT:
	    buf[0] = ' ';
	    strcpy(&buf[1], num(obj->o_ac, 0, RING));
	otherwise:
	    return "";
    }
    return buf;
}
