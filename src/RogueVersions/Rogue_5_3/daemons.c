/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	4.15 (NMT from Berkeley 5.2) 8/25/83
 */

#include <curses.h>
#include "rogue.h"

/*
 * doctor:
 *	A healing daemon that restors hit points after rest
 */
doctor()
{
    register int lv, ohp;

    lv = pstats.s_lvl;
    ohp = pstats.s_hpt;
    quiet++;
    if (lv < 8)
    {
	if (quiet + (lv << 1) > 20)
	    pstats.s_hpt++;
    }
    else
	if (quiet >= 3)
	    pstats.s_hpt += rnd(lv - 7) + 1;
    if (ISRING(LEFT, R_REGEN))
	pstats.s_hpt++;
    if (ISRING(RIGHT, R_REGEN))
	pstats.s_hpt++;
    if (ohp != pstats.s_hpt)
    {
	if (pstats.s_hpt > max_hp)
	    pstats.s_hpt = max_hp;
	quiet = 0;
    }
}

/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */
swander()
{
    daemon(rollwand, 0, BEFORE);
}

/*
 * rollwand:
 *	Called to roll to see if a wandering monster starts up
 */
rollwand()
{
    static int between = 0;

    if (++between >= 4)
    {
	if (roll(1, 6) == 4)
	{
	    wanderer();
	    kill_daemon(rollwand);
	    fuse(swander, 0, WANDERTIME, BEFORE);
	}
	between = 0;
    }
}

/*
 * unconfuse:
 *	Release the poor player from his confusion
 */
unconfuse()
{
    player.t_flags &= ~ISHUH;
    msg("you feel less %s now", on(player, ISTrip) ? "trippy" : "confused");
}

/*
 * unsee:
 *	Turn off the ability to see invisible
 */
unsee()
{
    register THING *th;

    for (th = mlist; th != NULL; th = next(th))
	if (on(*th, ISINVIS) && see_monst(th))
	{
	    move(th->t_pos.y, th->t_pos.x);
	    addrawch(th->t_oldch);
	}
    player.t_flags &= ~CANSEE;
}

/*
 * sight:
 *	He gets his sight back
 */
sight()
{
    if (on(player, ISBLIND))
    {
	extinguish(sight);
	player.t_flags &= ~ISBLIND;
	if (!(proom->r_flags & ISGONE))
	    enter_room(&hero);
	if (on(player, ISTrip))
	    msg("far out!  Everything is all cosmic again");
	else
	    msg("the veil of darkness lifts");
    }
}

/*
 * nohaste:
 *	End the hasting
 */
nohaste()
{
    player.t_flags &= ~ISHASTE;
    msg("you feel yourself slowing down");
}

/*
 * stomach:
 *	Digest the hero's food
 */
stomach()
{
    register int oldfood;
    //MDK_LOG("status: food_left=%d\n", food_left);
    if (food_left <= 0)
    {
	if (food_left-- < -STARVETIME)
	    death('s');
	/*
	 * the hero is fainting
	 */
	if (no_command || rnd(5) != 0)
	    return;
	no_command += rnd(8) + 4;
	player.t_flags &= ~ISRUN;
	running = FALSE;
	count = 0;
	hungry_state = 3;
	if (on(player, ISTrip)) {
	    if (!terse)
		addmsg("the munchies overpower your motor capabilities.  ");
	    msg("You freak out");
	}
	else {
	    if (!terse)
		addmsg("you feel too weak from lack of food.  ");
	    msg("You faint");
	}
    }
    else
    {
	oldfood = food_left;
	food_left -= ring_eat(LEFT) + ring_eat(RIGHT) + 1 - amulet;

	if (food_left < MORETIME && oldfood >= MORETIME)
	{
	    hungry_state = 2;
	    if (on(player, ISTrip))
		msg("the munchies are interfering with your motor capabilites");
	    else
		msg("you are starting to feel weak");
	}
	else if (food_left < 2 * MORETIME && oldfood >= 2 * MORETIME)
	{
	    hungry_state = 1;
	    if (on(player, ISTrip)) {
		if (!terse)
		    addmsg("you are ");
		msg("getting the munchies");
	    }
	    else
		if (!terse)
		    msg("you are starting to get hungry");
		else
		    msg("getting hungry");
	}
    }
}

/*
 * come_down:
 *	Take the hero down off her acid trip.
 */
come_down()
{
    register THING *tp;
    register bool seemonst;

    if (!on(player, ISTrip))
	return;

    kill_daemon(visuals);

    if (on(player, ISBLIND))
	return;

    /*
     * undo the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
	if (cansee(tp->o_pos.y, tp->o_pos.x))
	    mvaddrawch(tp->o_pos.y, tp->o_pos.x, tp->o_type);

    /*
     * undo the stairs
     */
    if (seenstairs)
	mvaddrawch(stairs.y, stairs.x, STAIRS);

    /*
     * undo the monsters
     */
    seemonst = on(player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
	if (cansee(tp->t_pos.y, tp->t_pos.x))
	    if (!on(*tp, ISINVIS) || on(player, CANSEE))
		mvaddrawch(tp->t_pos.y, tp->t_pos.x, tp->t_disguise);
	    else
		mvaddrawch(tp->t_pos.y, tp->t_pos.x, chat(tp->t_pos.y, tp->t_pos.x));
	else if (seemonst)
	{
	    standout();
	    mvaddrawch(tp->t_pos.y, tp->t_pos.x, tp->t_type);
	    standend();
	}
    player.t_flags &= ~ISTrip;
    msg("Everything looks SO boring now.");
}

/*
 * visuals:
 *	change the characters for the player
 */
visuals()
{
    register THING *tp;
    register bool seemonst;

    if (!after)
	return;
    /*
     * change the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
	if (cansee(tp->o_pos.y, tp->o_pos.x)) {
        PC_GFX_PASSGE_COLOR(tp->o_pos.y, tp->o_pos.x, 0, 0x70);
	    mvaddrawch(tp->o_pos.y, tp->o_pos.x, rnd_thing());
        PC_GFX_NOCOLOR(0x70);
    }

    /*
     * change the stairs
     */
    if (!seenstairs && cansee(stairs.y, stairs.x)) {
        PC_GFX_PASSGE_COLOR(stairs.y, stairs.x, 0, 0x70);
        mvaddrawch(stairs.y, stairs.x, rnd_thing());
        PC_GFX_NOCOLOR(0x70);
    }

    /*
     * change the monsters
     */
    seemonst = on(player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
	if (see_monst(tp)){
        PC_GFX_PASSGE_COLOR(tp->t_pos.y, tp->t_pos.x, 0, 0x70);
	    if (tp->t_type == 'M' && tp->t_disguise != 'M')
		mvaddrawch(tp->t_pos.y, tp->t_pos.x, rnd_thing());
	    else
		mvaddrawch(tp->t_pos.y, tp->t_pos.x, rnd(26) + 'A');
        PC_GFX_NOCOLOR(0x70);
	}
	else if (seemonst)
	{
	    standout();
	    mvaddrawch(tp->t_pos.y, tp->t_pos.x, rnd(26) + 'A');
	    standend();
	}
}
