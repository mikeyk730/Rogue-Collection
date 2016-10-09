/*
 * Code for one creature to chase another
 *
 * @(#)chase.c	4.25 (Berkeley) 5/5/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

#define DRAGONSHOT  5	/* one chance in DRAGONSHOT that a dragon will flame */

coord ch_ret;				/* Where chasing takes you */

/*
 * runners:
 *	Make all the running monsters move.
 */
runners()
{
    register THING *tp;
	register THING *ntp;

    for (tp = mlist; tp != NULL; tp = ntp)
    {
	ntp = next(tp);
	if (!on(*tp, ISHELD) && on(*tp, ISRUN))
	{
	    if (!on(*tp, ISSLOW) || tp->t_turn)
		if (do_chase(tp) == -1)
			continue;
	    if (on(*tp, ISHASTE))
		if (do_chase(tp) == -1)
			continue;
	    tp->t_turn ^= TRUE;
	}
    }
}

/*
 * do_chase:
 *	Make one thing chase another.
 */
do_chase(th)
register THING *th;
{
    register struct room *rer, *ree;	/* room of chaser, room of chasee */
    register int mindist = 32767, i, dist;
    register bool stoprun = FALSE;	/* TRUE means we are there */
    register char sch;
    register bool door;
    register THING *obj;
    register struct room *oroom;
    coord this;				/* Temporary destination for chaser */

    rer = th->t_room;		/* Find room of chaser */
    if (on(*th, ISGREED) && rer->r_goldval == 0)
	th->t_dest = &hero;	/* If gold has been taken, run after hero */
    if (th->t_dest == &hero)	/* Find room of chasee */
	ree = proom;
    else
	ree = roomin(th->t_dest);
    /*
     * We don't count doors as inside rooms for this routine
     */
    door = (chat(th->t_pos.y, th->t_pos.x) == DOOR);
    /*
     * If the object of our desire is in a different room,
     * and we are not in a corridor, run to the door nearest to
     * our goal.
     */
over:
    if (rer != ree)
    {
	for (i = 0; i < rer->r_nexits; i++)	/* loop through doors */
	{
	    dist = DISTANCE(th->t_dest->y, th->t_dest->x,
			    rer->r_exit[i].y, rer->r_exit[i].x);
	    if (dist < mindist)
	    {
		this = rer->r_exit[i];
		mindist = dist;
	    }
	}
	if (door)
	{
	    rer = &passages[flat(th->t_pos.y, th->t_pos.x) & F_PNUM];
	    door = FALSE;
	    goto over;
	}
    }
    else
    {
	this = *th->t_dest;
	/*
	 * For dragons check and see if (a) the hero is on a straight
	 * line from it, and (b) that it is within shooting distance,
	 * but outside of striking range.
	 */
	if (th->t_type == 'D' && (th->t_pos.y == hero.y || th->t_pos.x == hero.x
	    || abs(th->t_pos.y - hero.y) == abs(th->t_pos.x - hero.x))
	    && DISTANCE(th->t_pos.y, th->t_pos.x, hero.y, hero.x) <= BOLT_LENGTH * BOLT_LENGTH
	    && !on(*th, ISCANC) && rnd(DRAGONSHOT) == 0)
	{
	    delta.y = sign(hero.y - th->t_pos.y);
	    delta.x = sign(hero.x - th->t_pos.x);
	    fire_bolt(&th->t_pos, &delta, "flame");
	    running = FALSE;
	    count = quiet = 0;
	    return 0;
	}
    }
    /*
     * This now contains what we want to run to this time
     * so we run to it.  If we hit it we either want to fight it
     * or stop running
     */
    if (!chase(th, &this))
    {
	if (ce(this, hero))
	{
	    return ( attack(th) );
	}
	else if (ce(this, *th->t_dest))
	{
	    for (obj = lvl_obj; obj != NULL; obj = next(obj))
		if (th->t_dest == &obj->o_pos)
		{
		    detach(lvl_obj, obj);
		    attach(th->t_pack, obj);
		    chat(obj->o_pos.y, obj->o_pos.x) =
			(th->t_room->r_flags & ISGONE) ? PASSAGE : FLOOR;
		    th->t_dest = find_dest(th);
		    break;
		}
	    if (th->t_type != 'F')
		stoprun = TRUE;
	}
    }
    else if (th->t_type == 'F')
	return(0);
    mvaddch(th->t_pos.y, th->t_pos.x, th->t_oldch);
    if (!ce(ch_ret, th->t_pos))
    {
	sch = mvinch(ch_ret.y, ch_ret.x);
	if (sch == FLOOR && (th->t_room->r_flags & ISDARK)
	    && DISTANCE(th->t_pos.y, th->t_pos.x, hero.y, hero.x)
	    && !on(player, ISBLIND))
		th->t_oldch = ' ';
	else
	    th->t_oldch = sch;
	oroom = th->t_room;
	th->t_room = roomin(&ch_ret);
	if (oroom != th->t_room)
	    th->t_dest = find_dest(th);

	moat(th->t_pos.y, th->t_pos.x) = NULL;
	moat(ch_ret.y, ch_ret.x) = th;
	th->t_pos = ch_ret;
    }
    if (see_monst(th))
	mvaddch(ch_ret.y, ch_ret.x, th->t_disguise);
    else if (on(player, SEEMONST))
    {
	standout();
	mvaddch(ch_ret.y, ch_ret.x, th->t_type);
	standend();
    }
    /*
     * And stop running if need be
     */
    if (stoprun && ce(th->t_pos, *(th->t_dest)))
	th->t_flags &= ~ISRUN;
    
    return(0);
}

/*
 * see_monst:
 *	Return TRUE if the hero can see the monster
 */
see_monst(mp)
register THING *mp;
{
    if (on(player, ISBLIND))
	return FALSE;
    if (on(*mp, ISINVIS) && !on(player, CANSEE))
	return FALSE;
    if (DISTANCE(mp->t_pos.y, mp->t_pos.x, hero.y, hero.x) < LAMPDIST)
	return TRUE;
    if (mp->t_room != proom)
	return FALSE;
    return (!(mp->t_room->r_flags & ISDARK));
}

/*
 * runto:
 *	Set a mosnter running after something or stop it from running
 *	(for when it dies)
 */
runto(runner, spot)
register coord *runner;
coord *spot;
{
    register THING *tp;

    /*
     * If we couldn't find him, something is funny
     */
#ifdef WIZARD
    if ((tp = moat(runner->y, runner->x)) == NULL)
	msg("couldn't find monster in runto at (%d,%d)", runner->y, runner->x);
#else
    tp = moat(runner->y, runner->x);
#endif
    /*
     * Start the beastie running
     */
    if (tp == NULL)
        return;
    tp->t_flags |= ISRUN;
    tp->t_flags &= ~ISHELD;
    tp->t_dest = find_dest(tp);
}

/*
 * chase:
 *	Find the spot for the chaser(er) to move closer to the
 *	chasee(ee).  Returns TRUE if we want to keep on chasing later
 *	FALSE if we reach the goal.
 */
chase(tp, ee)
THING *tp;
coord *ee;
{
    register int x, y;
    register int dist, thisdist;
    register THING *obj;
    register coord *er = &tp->t_pos;
    register char ch;
    register int plcnt = 1;

    /*
     * If the thing is confused, let it move randomly. Invisible
     * Stalkers are slightly confused all of the time, and bats are
     * quite confused all the time
     */
    if ((on(*tp, ISHUH) && rnd(5) != 0) || (tp->t_type == 'I' && rnd(5) == 0)
	|| (tp->t_type == 'B' && rnd(2) == 0))
    {
	/*
	 * get a valid random move
	 */
	ch_ret = *rndmove(tp);
	dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);
	/*
	 * Small chance that it will become un-confused 
	 */
	if (rnd(20) == 0)
	    tp->t_flags &= ~ISHUH;
    }
    /*
     * Otherwise, find the empty spot next to the chaser that is
     * closest to the chasee.
     */
    else
    {
	register int ey, ex;
	/*
	 * This will eventually hold where we move to get closer
	 * If we can't find an empty spot, we stay where we are.
	 */
	dist = DISTANCE(er->y, er->x, ee->y, ee->x);
	ch_ret = *er;

	ey = er->y + 1;
	ex = er->x + 1;
	for (x = er->x - 1; x <= ex; x++)
	    for (y = er->y - 1; y <= ey; y++)
	    {
		coord tryp;

		tryp.x = x;
		tryp.y = y;
		if (!diag_ok(er, &tryp))
		    continue;
		ch = winat(y, x);
		if (step_ok(ch))
		{
		    /*
		     * If it is a scroll, it might be a scare monster scroll
		     * so we need to look it up to see what type it is.
		     */
		    if (ch == SCROLL)
		    {
			for (obj = lvl_obj; obj != NULL; obj = next(obj))
			{
			    if (y == obj->o_pos.y && x == obj->o_pos.x)
				break;
			}
			if (obj != NULL && obj->o_which == S_SCARE)
			    continue;
		    }
		    /*
		     * It can also be a Mimic, which we shouldn't step on
		     */
		    if ((obj = moat(y, x)) != NULL && obj->t_type == 'M')
		        continue;
		    /*
		     * If we didn't find any scrolls at this place or it
		     * wasn't a scare scroll, then this place counts
		     */
		    thisdist = DISTANCE(y, x, ee->y, ee->x);
		    if (thisdist < dist)
		    {
			plcnt = 1;
			ch_ret = tryp;
			dist = thisdist;
		    }
		    else if (thisdist == dist && rnd(++plcnt) == 0)
		    {
			ch_ret = tryp;
			dist = thisdist;
		    }
		}
	    }
    }
    return (dist != 0 && !ce(ch_ret, hero));
}

/*
 * roomin:
 *	Find what room some coordinates are in. NULL means they aren't
 *	in any room.
 */
struct room *
roomin(cp)
register coord *cp;
{
    register struct room *rp;
    register char *fp;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	if (cp->x < rp->r_pos.x + rp->r_max.x && rp->r_pos.x <= cp->x
	 && cp->y < rp->r_pos.y + rp->r_max.y && rp->r_pos.y <= cp->y)
	    return rp;
    fp = &flat(cp->y, cp->x);
    if (*fp & F_PASS)
	return &passages[*fp & F_PNUM];
    msg("in some bizarre place (%d, %d)", unc(*cp));
    return NULL;
}

/*
 * diag_ok:
 *	Check to see if the move is legal if it is diagonal
 */
diag_ok(sp, ep)
register coord *sp, *ep;
{
    if (ep->x == sp->x || ep->y == sp->y)
	return TRUE;
    return (step_ok(chat(ep->y, sp->x)) && step_ok(chat(sp->y, ep->x)));
}

/*
 * cansee:
 *	Returns true if the hero can see a certain coordinate.
 */
cansee(y, x)
register int y, x;
{
    register struct room *rer;
    coord tp;

    if (on(player, ISBLIND))
	return FALSE;
    if (DISTANCE(y, x, hero.y, hero.x) < LAMPDIST)
	return TRUE;
    /*
     * We can only see if the hero in the same room as
     * the coordinate and the room is lit or if it is close.
     */
    tp.y = y;
    tp.x = x;
    return ((rer = roomin(&tp)) == proom && !(rer->r_flags & ISDARK));
}

/*
 * find_dest:
 *	find the proper destination for the monster
 */
coord *
find_dest(tp)
register THING *tp;
{
    register THING *obj;
    register int prob;
    register struct room *rp;

    if ((prob = monsters[tp->t_type - 'A'].m_carry) <= 0 || tp->t_room == proom
	|| see_monst(tp))
	    return &hero;
    rp = tp->t_room;
    for (obj = lvl_obj; obj != NULL; obj = next(obj))
    {
	if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	    continue;
	if (roomin(&obj->o_pos) == rp && rnd(100) < prob)
	{
	    for (tp = mlist; tp != NULL; tp = next(tp))
		if (tp->t_dest == &obj->o_pos)
		    break;
	    if (tp == NULL)
		return &obj->o_pos;
	}
    }
    return &hero;
}
