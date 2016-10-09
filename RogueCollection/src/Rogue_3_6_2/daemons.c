/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	3.7 (Berkeley) 6/15/81
 */

#include "curses.h"
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
	if (quiet > 20 - lv*2)
	    pstats.s_hpt++;
    }
    else
	if (quiet >= 3)
	    pstats.s_hpt += rnd(lv - 7)+1;
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

int between = 0;

rollwand()
{
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
    msg("You feel less confused now");
}


/*
 * unsee:
 *	He lost his see invisible power
 */

unsee()
{
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
	light(&hero);
	msg("The veil of darkness lifts");
    }
}

/*
 * nohaste:
 *	End the hasting
 */

nohaste()
{
    player.t_flags &= ~ISHASTE;
    msg("You feel yourself slowing down.");
}

/*
 * digest the hero's food
 */
stomach()
{
    register int oldfood;

    if (food_left <= 0)
    {
	/*
	 * the hero is fainting
	 */
	if (no_command || rnd(100) > 20)
	    return;
	no_command = rnd(8)+4;
	if (!terse)
	    addmsg("You feel too weak from lack of food.  ");
	msg("You faint");
	running = FALSE;
	count = 0;
	hungry_state = 3;
    }
    else
    {
	oldfood = food_left;
	food_left -= ring_eat(LEFT) + ring_eat(RIGHT) + 1 - amulet;

	if (food_left < MORETIME && oldfood >= MORETIME)
	{
	    msg("You are starting to feel weak");
	    hungry_state = 2;
	}
	else if (food_left < 2 * MORETIME && oldfood >= 2 * MORETIME)
	{
	    if (!terse)
		msg("You are starting to get hungry");
	    else
		msg("Getting hungry");
	    hungry_state = 1;
	}
    }
}
