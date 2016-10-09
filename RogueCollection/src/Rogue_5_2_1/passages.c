/*
 * Draw the connecting passages
 *
 * @(#)passages.c	4.8 (Berkeley) 1/27/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

/*
 * do_passages:
 *	Draw all the passages on a level.
 */
do_passages()
{
    register struct rdes *r1, *r2 = NULL;
    register int i, j;
    register int roomcount;
    static struct rdes
    {
	bool	conn[MAXROOMS];		/* possible to connect to room i? */
	bool	isconn[MAXROOMS];	/* connection been made to room i? */
	bool	ingraph;		/* this room in graph already? */
    } rdes[MAXROOMS] = {
	{ { 0, 1, 0, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 1, 0, 1, 0, 1, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 1, 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 1, 0, 0, 0, 1, 0, 1, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 1, 0, 1, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 1, 0, 1, 0, 0, 0, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 0, 1, 0, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 0, 0, 1, 0, 1, 0, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 0, 0, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
    };

    /*
     * reinitialize room graph description
     */
    for (r1 = rdes; r1 <= &rdes[MAXROOMS-1]; r1++)
    {
	for (j = 0; j < MAXROOMS; j++)
	    r1->isconn[j] = FALSE;
	r1->ingraph = FALSE;
    }

    /*
     * starting with one room, connect it to a random adjacent room and
     * then pick a new room to start with.
     */
    roomcount = 1;
    r1 = &rdes[rnd(MAXROOMS)];
    r1->ingraph = TRUE;
    do
    {
	/*
	 * find a room to connect with
	 */
	j = 0;
	for (i = 0; i < MAXROOMS; i++)
	    if (r1->conn[i] && !rdes[i].ingraph && rnd(++j) == 0)
		r2 = &rdes[i];
	/*
	 * if no adjacent rooms are outside the graph, pick a new room
	 * to look from
	 */
	if (j == 0)
	{
	    do
		r1 = &rdes[rnd(MAXROOMS)];
	    until (r1->ingraph);
	}
	/*
	 * otherwise, connect new room to the graph, and draw a tunnel
	 * to it
	 */
	else
	{
	    r2->ingraph = TRUE;
	    i = r1 - rdes;
	    j = r2 - rdes;
	    conn(i, j);
	    r1->isconn[j] = TRUE;
	    r2->isconn[i] = TRUE;
	    roomcount++;
	}
    } while (roomcount < MAXROOMS);

    /*
     * attempt to add passages to the graph a random number of times so
     * that there isn't always just one unique passage through it.
     */
    for (roomcount = rnd(5); roomcount > 0; roomcount--)
    {
	r1 = &rdes[rnd(MAXROOMS)];	/* a random room to look from */
	/*
	 * find an adjacent room not already connected
	 */
	j = 0;
	for (i = 0; i < MAXROOMS; i++)
	    if (r1->conn[i] && !r1->isconn[i] && rnd(++j) == 0)
		r2 = &rdes[i];
	/*
	 * if there is one, connect it and look for the next added
	 * passage
	 */
	if (j != 0)
	{
	    i = r1 - rdes;
	    j = r2 - rdes;
	    conn(i, j);
	    r1->isconn[j] = TRUE;
	    r2->isconn[i] = TRUE;
	}
    }
    passnum();
}

/*
 * conn:
 *	Draw a corridor from a room in a certain direction.
 */
conn(r1, r2)
int r1, r2;
{
    register struct room *rpf, *rpt = NULL;
    register char rmt;
    register int distance = 0, turn_spot = 0, turn_distance = 0, index;
    register int rm;
    register char direc;
    coord del = {0,0}, curr, turn_delta = {0,0}, spos = {0,0}, epos = {0,0};

    if (r1 < r2)
    {
	rm = r1;
	if (r1 + 1 == r2)
	    direc = 'r';
	else
	    direc = 'd';
    }
    else
    {
	rm = r2;
	if (r2 + 1 == r1)
	    direc = 'r';
	else
	    direc = 'd';
    }
    rpf = &rooms[rm];
    /*
     * Set up the movement variables, in two cases:
     * first drawing one down.
     */
    if (direc == 'd')
    {
	rmt = rm + 3;				/* room # of dest */
	rpt = &rooms[rmt];			/* room pointer of dest */
	del.x = 0;				/* direction of move */
	del.y = 1;
	spos.x = rpf->r_pos.x;			/* start of move */
	spos.y = rpf->r_pos.y;
	epos.x = rpt->r_pos.x;			/* end of move */
	epos.y = rpt->r_pos.y;
	if (!(rpf->r_flags & ISGONE))		/* if not gone pick door pos */
	{
	    spos.x += rnd(rpf->r_max.x - 2) + 1;
	    spos.y += rpf->r_max.y - 1;
	}
	if (!(rpt->r_flags & ISGONE))
	    epos.x += rnd(rpt->r_max.x - 2) + 1;
	distance = abs(spos.y - epos.y) - 1;	/* distance to move */
	turn_delta.y = 0;			/* direction to turn */
	turn_delta.x = (spos.x < epos.x ? 1 : -1);
	turn_distance = abs(spos.x - epos.x);	/* how far to turn */
	turn_spot = rnd(distance-1) + 1;		/* where turn starts */
    }
    else if (direc == 'r')			/* setup for moving right */
    {
	rmt = rm + 1;
	rpt = &rooms[rmt];
	del.x = 1;
	del.y = 0;
	spos.x = rpf->r_pos.x;
	spos.y = rpf->r_pos.y;
	epos.x = rpt->r_pos.x;
	epos.y = rpt->r_pos.y;
	if (!(rpf->r_flags & ISGONE))
	{
	    spos.x += rpf->r_max.x-1;
	    spos.y += rnd(rpf->r_max.y-2)+1;
	}
	if (!(rpt->r_flags & ISGONE))
	    epos.y += rnd(rpt->r_max.y-2)+1;
	distance = abs(spos.x - epos.x) - 1;
	turn_delta.y = (spos.y < epos.y ? 1 : -1);
	turn_delta.x = 0;
	turn_distance = abs(spos.y - epos.y);
	turn_spot = rnd(distance-1) + 1;
    }
#ifdef WIZARD
    else
	debug("error in connection tables");
#endif
    /*
     * Draw in the doors on either side of the passage or just put #'s
     * if the rooms are gone.
     */
    if (!(rpf->r_flags & ISGONE))
	door(rpf, &spos);
    else
    {
	index = INDEX(spos.y, spos.x);
	_level[index] = PASSAGE;
	_flags[index] |= F_PASS;
    }
    if (!(rpt->r_flags & ISGONE))
	door(rpt, &epos);
    else
    {
	index = INDEX(epos.y, epos.x);
	_level[index] = PASSAGE;
	_flags[index] |= F_PASS;
    }
    /*
     * Get ready to move...
     */
    curr.x = spos.x;
    curr.y = spos.y;
    while (distance)
    {
	/*
	 * Move to new position
	 */
	curr.x += del.x;
	curr.y += del.y;
	/*
	 * Check if we are at the turn place, if so do the turn
	 */
	if (distance == turn_spot)
	    while (turn_distance--)
	    {
		index = INDEX(curr.y, curr.x);
		_level[index] = PASSAGE;
		_flags[index] |= F_PASS;
		curr.x += turn_delta.x;
		curr.y += turn_delta.y;
	    }
	/*
	 * Continue digging along
	 */
	index = INDEX(curr.y, curr.x);
	_level[index] = PASSAGE;
	_flags[index] |= F_PASS;
	distance--;
    }
    curr.x += del.x;
    curr.y += del.y;
    if (!ce(curr, epos))
	msg("warning, connectivity problem on this level");
}

/*
 * door:
 *	Add a door or possibly a secret door.  Also enters the door in
 *	the exits array of the room.
 */
door(rm, cp)
register struct room *rm;
register coord *cp;
{
    register int index;

    index = INDEX(cp->y, cp->x);
    if (rnd(10) + 1 < level && rnd(5) == 0)
    {
	_level[index] = (cp->y == rm->r_pos.y || cp->y == rm->r_pos.y + rm->r_max.y - 1) ? '-' : '|';
	_flags[index] &= ~F_REAL;
    }
    else
	_level[index] = DOOR;
    rm->r_exit[rm->r_nexits++] = *cp;
}

#ifdef WIZARD
/*
 * add_pass:
 *	Add the passages to the current window (wizard command)
 */
add_pass()
{
    register int y, x, ch;

    for (y = 1; y < LINES - 1; y++)
	for (x = 0; x < COLS; x++)
	    if ((ch = chat(y, x)) == DOOR || ch == PASSAGE)
		mvaddch(y, x, ch);
}
#endif

/*
 * passnum:
 *	Assign a number to each passageway
 */
static int pnum;
static bool newpnum;

passnum()
{
    register struct room *rp;
    register int i;

    pnum = 0;
    newpnum = FALSE;
    for (rp = passages; rp < &passages[MAXPASS]; rp++)
	rp->r_nexits = 0;
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	for (i = 0; i < rp->r_nexits; i++)
	{
	    newpnum++;
	    numpass(rp->r_exit[i].y, rp->r_exit[i].x);
	}
}

/*
 * numpass:
 *	Number a passageway square and its brethren
 */
numpass(y, x)
register int y, x;
{
    register char *fp;
    register struct room *rp;
    register char ch;

    fp = &flat(y, x);
    if (*fp & F_PNUM)
	return;
    if (newpnum)
    {
	pnum++;
	newpnum = FALSE;
    }
    /*
     * check to see if it is a door or secret door, i.e., a new exit,
     * or a numerable type of place
     */
    if ((ch = chat(y, x)) == DOOR || (!(*fp & F_REAL) && ch != FLOOR))
    {
	rp = &passages[pnum];
	rp->r_exit[rp->r_nexits].y = y;
	rp->r_exit[rp->r_nexits++].x = x;
    }
    else if (!(*fp & F_PASS))
	return;
    *fp |= pnum;
    /*
     * recurse on the surrounding places
     */
    numpass(y + 1, x);
    numpass(y - 1, x);
    numpass(y, x + 1);
    numpass(y, x - 1);
}
