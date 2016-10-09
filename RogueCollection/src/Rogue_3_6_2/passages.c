/*
 * Draw the connecting passages
 *
 * @(#)passages.c	3.4 (Berkeley) 6/15/81
 */

#include "curses.h"
#include "rogue.h"

/*
 * do_passages:
 *	Draw all the passages on a level.
 */

do_passages()
{
    register struct rdes *r1, *r2;
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
     * that there isn't just one unique passage through it.
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
}

/*
 * conn:
 *	Draw a corridor from a room in a certain direction.
 */

conn(r1, r2)
int r1, r2;
{
    register struct room *rpf, *rpt;
    register char rmt;
    register int distance, turn_spot, turn_distance;
    register int rm;
    register char direc;
    coord delta, curr, turn_delta, spos, epos;

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
	delta.x = 0;				/* direction of move */
	delta.y = 1;
	spos.x = rpf->r_pos.x;			/* start of move */
	spos.y = rpf->r_pos.y;
	epos.x = rpt->r_pos.x;			/* end of move */
	epos.y = rpt->r_pos.y;
	if (!(rpf->r_flags & ISGONE))		/* if not gone pick door pos */
	{
	    spos.x += rnd(rpf->r_max.x-2)+1;
	    spos.y += rpf->r_max.y-1;
	}
	if (!(rpt->r_flags & ISGONE))
	    epos.x += rnd(rpt->r_max.x-2)+1;
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
	delta.x = 1;
	delta.y = 0;
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
    else
	debug("error in connection tables");
    /*
     * Draw in the doors on either side of the passage or just put #'s
     * if the rooms are gone.
     */
    if (!(rpf->r_flags & ISGONE)) door(rpf, &spos);
    else
    {
	cmov(spos);
	addch('#');
    }
    if (!(rpt->r_flags & ISGONE)) door(rpt, &epos);
    else
    {
	cmov(epos);
	addch('#');
    }
    /*
     * Get ready to move...
     */
    curr.x = spos.x;
    curr.y = spos.y;
    while(distance)
    {
	/*
	 * Move to new position
	 */
	curr.x += delta.x;
	curr.y += delta.y;
	/*
	 * Check if we are at the turn place, if so do the turn
	 */
	if (distance == turn_spot && turn_distance > 0)
	    while(turn_distance--)
	    {
		cmov(curr);
		addch(PASSAGE);
		curr.x += turn_delta.x;
		curr.y += turn_delta.y;
	    }
	/*
	 * Continue digging along
	 */
	cmov(curr);
	addch(PASSAGE);
	distance--;
    }
    curr.x += delta.x;
    curr.y += delta.y;
    if (!ce(curr, epos))
	msg("Warning, connectivity problem on this level.");
}

/*
 * Add a door or possibly a secret door
 * also enters the door in the exits array of the room.
 */

door(rm, cp)
register struct room *rm;
register coord *cp;
{
    cmov(*cp);
    addch(rnd(10) < level - 1 && rnd(100) < 20 ? SECRETDOOR : DOOR);
    rm->r_exit[rm->r_nexits++] = *cp;
}
/*
 * add_pass:
 *	add the passages to the current window (wizard command)
 */

add_pass()
{
    register int y, x, ch;

    for (y = 1; y < LINES - 2; y++)
	for (x = 0; x < COLS; x++)
	    if ((ch=mvinch(y, x)) == PASSAGE || ch == DOOR || ch == SECRETDOOR)
		mvwaddch(cw, y, x, ch);
}
