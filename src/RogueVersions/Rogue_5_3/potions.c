/*
 * Function(s) for dealing with potions
 *
 * @(#)potions.c	4.30 (NMT from Berkeley 5.2) 8/25/83
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * quaff:
 *	Quaff a potion from the pack
 */
quaff()
{
    register THING *obj, *th;
    register bool discardit = FALSE;

    obj = get_item("quaff", POTION);
    /*
     * Make certain that it is somethings that we want to drink
     */
    if (obj == NULL)
	return;
    if (obj->o_type != POTION)
    {
	if (!terse)
	    msg("yuk! Why would you want to drink that?");
	else
	    msg("that's undrinkable");
	return;
    }
    if (obj == cur_weapon)
	cur_weapon = NULL;
    play_sound("eat");

    /*
     * Calculate the effect it has on the poor guy.
     */
    del_obj = obj;
    switch (obj->o_which)
    {
	when P_CONFUSE:
	    p_know[P_CONFUSE] = TRUE;
	    if (!on(player, ISHUH))
	    {
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8)+HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8)+HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
		if (on(player, ISTrip))
		    msg("what a tripy feeling!");
		else
		    msg("wait, what's going on here. Huh? What? Who?");
	    }
	when P_POISON:
	    p_know[P_POISON] = TRUE;
	    if (!ISWEARING(R_SUSTSTR))
	    {
		chg_str(-(rnd(3)+1));
		msg("you feel very sick now");
		come_down();
	    }
	    else
		msg("you feel momentarily sick");
	when P_HEALING:
	    p_know[P_HEALING] = TRUE;
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    sight();
	    msg("you begin to feel better");
	when P_STRENGTH:
	    p_know[P_STRENGTH] = TRUE;
	    chg_str(1);
	    msg("you feel stronger, now.  What bulging muscles!");
	when P_MFIND:
	    player.t_flags |= SEEMONST;
	    fuse(turn_see, TRUE, HUHDURATION, AFTER);
	    if (!turn_see(FALSE))
		msg("you have a %s feeling for a moment, then it passes",
		    on(player, ISTrip) ? "normal" : "strange");
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    if (lvl_obj != NULL)
	    {
		register THING *tp;
		register bool show;

		show = FALSE;
		wclear(hw);
		for (tp = lvl_obj; tp != NULL; tp = next(tp))
		{
		    if (is_magic(tp))
		    {
			show = TRUE;
			mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, MAGIC);
			p_know[P_TFIND] = TRUE;
		    }
		}
		for (th = mlist; th != NULL; th = next(th))
		{
		    for (tp = th->t_pack; tp != NULL; tp = next(tp))
		    {
			if (is_magic(tp))
			{
			    show = TRUE;
			    mvwaddch(hw, th->t_pos.y, th->t_pos.x, MAGIC);
			    p_know[P_TFIND] = TRUE;
			}
		    }
		}
		if (show)
		{
		    show_win(hw, 
			"You sense the presence of magic on this level.--More--");
		    break;
		}
	    }
	    msg("you have a %s feeling for a moment, then it passes",
		on(player, ISTrip) ? "normal" : "strange");
	when P_LSD:
	    p_know[P_LSD] = TRUE;
	    if (!on(player, ISTrip))
	    {
		player.t_flags |= ISTrip;
		fuse(come_down, 0, SEEDURATION, AFTER);
		daemon(visuals, 0, AFTER);
		if (on(player, SEEMONST))
		    turn_see(FALSE);
		seenstairs = seen_stairs();
	    }
	    else
		lengthen(come_down, SEEDURATION);
	    msg("Oh, wow!  Everything seems so cosmic!");
	when P_SEEINVIS:
	    if (on(player, CANSEE))
		lengthen(unsee, SEEDURATION);
	    else
	    {
		fuse(unsee, 0, SEEDURATION, AFTER);
		invis_on();
		look(FALSE);
	    }
	    sight();
	    msg("this potion tastes like %s juice", fruit);
	when P_RAISE:
	    p_know[P_RAISE] = TRUE;
	    msg("you suddenly feel much more skillful");
	    raise_level();
	when P_XHEAL:
	    p_know[P_XHEAL] = TRUE;
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > max_hp)
	    {
		if (pstats.s_hpt > max_hp + pstats.s_lvl + 1)
		    ++max_hp;
		pstats.s_hpt = ++max_hp;
	    }
	    sight();
	    come_down();
	    msg("you begin to feel much better");
	when P_HASTE:
	    p_know[P_HASTE] = TRUE;
	    if (add_haste(TRUE))
		msg("you feel yourself moving much faster");
	when P_RESTORE:
	    if (ISRING(LEFT, R_ADDSTR))
		add_str(&pstats.s_str, -cur_ring[LEFT]->o_ac);
	    if (ISRING(RIGHT, R_ADDSTR))
		add_str(&pstats.s_str, -cur_ring[RIGHT]->o_ac);
	    if (pstats.s_str < max_stats.s_str)
		pstats.s_str = max_stats.s_str;
	    if (ISRING(LEFT, R_ADDSTR))
		add_str(&pstats.s_str, cur_ring[LEFT]->o_ac);
	    if (ISRING(RIGHT, R_ADDSTR))
		add_str(&pstats.s_str, cur_ring[RIGHT]->o_ac);
	    msg("hey, this tastes great.  It make you feel warm all over");
	when P_BLIND:
	    p_know[P_BLIND] = TRUE;
	    if (!on(player, ISBLIND))
	    {
		player.t_flags |= ISBLIND;
		fuse(sight, 0, SEEDURATION, AFTER);
		look(FALSE);
	    }
	    else
		lengthen(sight, SEEDURATION);
	    if (on(player, ISTrip))
		msg("oh, bummer!  Everything is dark!  Help!");
	    else
		msg("a cloak of darkness falls around you");
	when P_NOP:
	    if (on(player, ISTrip))
		msg("this potion tastes pretty");
	    else
		msg("this potion tastes extremely dull");
	otherwise:
	    msg("what an odd tasting potion!");
	    return;
    }
    status();
    /*
     * Throw the item away
     */
    inpack--;
    if (obj->o_count > 1)
	obj->o_count--;
    else
    {
	detach(pack, obj);
        discardit = TRUE;
    }

    call_it(p_know[obj->o_which], &p_guess[obj->o_which]);

    if (discardit)
	discard(obj);
    del_obj = NULL;
}

/*
 * invis_on:
 *	Turn on the ability to see invisible
 */
invis_on()
{
    register THING *th;

    player.t_flags |= CANSEE;
    for (th = mlist; th != NULL; th = next(th))
	if (on(*th, ISINVIS) && see_monst(th) && !on(player, ISTrip))
	{
	    move(th->t_pos.y, th->t_pos.x);
	    addrawch(th->t_disguise);
	}
}

/*
 * turn_see:
 *	Put on or off seeing monsters on this level
 */
turn_see(turn_off)
register bool turn_off;
{
    register THING *mp;
    register bool can_see, add_new;

    add_new = FALSE;
    for (mp = mlist; mp != NULL; mp = next(mp))
    {
	move(mp->t_pos.y, mp->t_pos.x);
	can_see = see_monst(mp);
	if (turn_off)
	{
	    if (!can_see)
		addrawch(mp->t_oldch);
	}
	else
	{
	    if (!can_see)
		standout();
	    if (!on(player, ISTrip))
		addrawch(mp->t_type);
	    else
		addrawch(rnd(26) + 'A');
	    if (!can_see)
	    {
		standend();
		add_new++;
	    }
	}
    }
    if (turn_off)
	player.t_flags &= ~SEEMONST;
    else
	player.t_flags |= SEEMONST;
    return add_new;
}

/*
 * seen_stairs:
 *	Return TRUE if the player has seen the stairs
 */
seen_stairs()
{
    register THING	*tp;

    if (MVINCH(stairs.y, stairs.x) == STAIRS)	/* it's on the map */
	return TRUE;
    if (ce(hero, stairs))			/* It's under him */
	return TRUE;

    /*
     * if a monster is on the stairs, this gets hairy
     */
    if ((tp = moat(stairs.y, stairs.x)) != NULL)
    {
	if (see_monst(tp) && on(*tp, ISRUN))	/* if it's visible and awake */
	    return TRUE;			/* it must have moved there */

	if (on(player, SEEMONST)		/* if she can detect monster */
	    && tp->t_oldch == STAIRS)		/* and there once were stairs */
		return TRUE;			/* it must have moved there */
    }
    return FALSE;
}
