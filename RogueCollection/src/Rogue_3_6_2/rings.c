#include "curses.h"
#include <stdlib.h>
#include "rogue.h"

/*
 * routines dealing specifically with rings
 *
 * @(#)rings.c	3.17 (Berkeley) 6/15/81
 */

ring_on()
{
    register struct object *obj;
    register struct linked_list *item;
    register int ring;
    str_t save_max;
    char buf[80];

    item = get_item("put on", RING);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != RING)
    {
	if (!terse)
	    msg("It would be difficult to wrap that around a finger");
	else
	    msg("Not a ring");
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
	    msg("You already have a ring on each hand");
	else
	    msg("Wearing two");
	return;
    }
    cur_ring[ring] = obj;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (obj->o_which)
    {
	case R_ADDSTR:
	    save_max = max_stats.s_str;
	    chg_str(obj->o_ac);
	    max_stats.s_str = save_max;
	    break;
	case R_SEEINVIS:
	    player.t_flags |= CANSEE;
	    light(&hero);
	    mvwaddch(cw, hero.y, hero.x, PLAYER);
	    break;
	case R_AGGR:
	    aggravate();
	    break;
    }
    status();
    if (r_know[obj->o_which] && r_guess[obj->o_which])
    {
	free(r_guess[obj->o_which]);
	r_guess[obj->o_which] = NULL;
    }
    else if (!r_know[obj->o_which] && askme && r_guess[obj->o_which] == NULL)
    {
	mpos = 0;
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (get_str(buf, cw) == NORM)
	{
	    r_guess[obj->o_which] = malloc((unsigned int) strlen(buf) + 1);
	    strcpy(r_guess[obj->o_which], buf);
	}
	msg("");
    }
}

ring_off()
{
    register int ring;
    register struct object *obj;

    if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL)
    {
	if (terse)
	    msg("No rings");
	else
	    msg("You aren't wearing any rings");
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
	msg("Not wearing such a ring");
	return;
    }
    if (dropcheck(obj))
	msg("Was wearing %s", inv_name(obj, TRUE));
}

gethand()
{
    register int c;

    for (;;)
    {
	if (terse)
	    msg("Left or Right ring? ");
	else
	    msg("Left hand or right hand? ");
	if ((c = readchar()) == 'l' || c == 'L')
	    return LEFT;
	else if (c == 'r' || c == 'R')
	    return RIGHT;
	else if (c == ESCAPE)
	    return -1;
	mpos = 0;
	if (terse)
	    msg("L or R");
	else
	    msg("Please type L or R");
    }
}

/*
 * how much food does this ring use up?
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
	    return 1;
	case R_SEARCH:
	    return (rnd(100) < 33);
	case R_DIGEST:
	    return -(rnd(100) < 50);
	default:
	    return 0;
    }
}

/*
 * print ring bonuses
 */
char *
ring_num(obj)
register struct object *obj;
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
	    strcpy(&buf[1], num(obj->o_ac, 0));
	otherwise:
	    return "";
    }
    return buf;
}
