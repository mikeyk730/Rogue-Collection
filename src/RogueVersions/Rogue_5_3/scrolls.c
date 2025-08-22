/*
 * Read a scroll and let it happen
 *
 * @(#)scrolls.c	4.24 (NMT from Berkeley 5.2) 8/25/83
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * read_scroll:
 *	Read a scroll from the pack and do the appropriate thing
 */
read_scroll()
{
    register THING *obj;
    register int y, x;
    register unsigned char ch;
    register THING *op;
    register int index, mcount;
    register bool discardit = FALSE;

    obj = get_item("read", SCROLL);
    if (obj == NULL)
	return;
    if (obj->o_type != SCROLL)
    {
	if (!terse)
	    msg("there is nothing on it to read");
	else
	    msg("nothing to read");
	return;
    }
    /*
     * Calculate the effect it has on the poor guy.
     */
    if (obj == cur_weapon)
	cur_weapon = NULL;
    del_obj = obj;
    switch (obj->o_which)
    {
	when S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    player.t_flags |= CANHUH;
	    addmsg("your hands begin to glow ");
	    if (on(player, ISTrip))
		msg(rnd_color());
	    else
		msg("red");
	when S_ARMOR:
	    if (cur_armor != NULL)
	    {
		cur_armor->o_ac--;
		cur_armor->o_flags &= ~ISCURSED;
		msg("your armor glows %s for a moment",
		    on(player, ISTrip) ? rnd_color() : "silver");
	    }
	when S_HOLD:
	    /*
	     * Hold monster scroll.  Stop all monsters within two spaces
	     * from chasing after the hero.
	     */

	    mcount = 0;
	    for (x = hero.x - 2; x <= hero.x + 2; x++)
		if (x >= 0 && x < COLS)
		    for (y = hero.y - 2; y <= hero.y + 2; y++)
			if (y >= 0 && y <= LINES - 1)
			    if ((op = moat(y, x)) != NULL && on(*op, ISRUN))
			    {
				op->t_flags &= ~ISRUN;
				op->t_flags |= ISHELD;
				mcount++;
			    }
	    if (mcount)
	    {
		addmsg("the monster");
		if (mcount > 1)
		    addmsg("s around you");
		addmsg(" freeze");
		if (mcount == 1)
		    addmsg("s");
		endmsg();
		s_know[S_HOLD] = TRUE;
	    }
	    else
		msg("you feel a strange sense of loss");
	when S_SLEEP:
	    /*
	     * Scroll which makes you fall asleep
	     */
	    s_know[S_SLEEP] = TRUE;
	    no_command += rnd(SLEEPTIME) + 4;
	    player.t_flags &= ~ISRUN;
	    msg("you fall asleep");
	when S_CREATE:
	    /*
	     * Create a monster
	     * First look in a circle around him, next try his room
	     * otherwise give up
	     */
	    {
		register bool appear = 0;
		coord mp;

		/*
		 * Search for an open place
		 */
		for (y = hero.y - 1; y <= hero.y + 1; y++)
		    for (x = hero.x - 1; x <= hero.x + 1; x++)
		    {
		    if (offmap(y, x)) continue; /* mdk: bounds check */
			/*
			 * Don't put a monster in top of the player.
			 */
			if (y == hero.y && x == hero.x)
			    continue;
			/*
			 * Or anything else nasty
			 */
			if (step_ok(ch = winat(y, x)))
			{
			    if (ch == SCROLL
				&& find_obj(y, x)->o_which == S_SCARE)
				    continue;
			    if (rnd(++appear) == 0)
			    {
				mp.y = y;
				mp.x = x;
			    }
			}
		    }
		if (appear)
		{
		    op = new_item();
		    new_monster(op, randmonster(FALSE), &mp);
		}
		else
		    msg("you hear a faint cry of anguish in the distance");
	    }
	when S_IDENT:
	    /*
	     * Identify, let the rogue figure something out
	     */
	    s_know[S_IDENT] = TRUE;
	    msg("this scroll is an identify scroll");
	    whatis(TRUE);
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    s_know[S_MAP] = TRUE;
	    msg("oh, now this scroll has a map on it");
	    /*
	     * Take all the things we want to keep hidden out of the window
	     */
	    for (y = 1; y < LINES - 1; y++)
		for (x = 0; x < COLS; x++)
		{
		    index = INDEX(y, x);
		    switch (ch = _level[index])
		    {
			case HWALL:
			case VWALL:
			    if (!(_flags[index] & F_REAL))
			    {
				ch = _level[index] = DOOR;
				_flags[index] &= ~F_REAL;
			    }
            PC_GFX_WALL_CASES
			case PASSAGE:
			    _flags[index] |= F_SEEN;
			case DOOR:
			case STAIRS:
			    if ((op = moat(y, x)) != NULL)
				if (op->t_oldch == ' ')
				    op->t_oldch = ch;
			    break;
			default:
			    ch = ' ';
		    }
		    if (ch != ' ')
			mvaddrawch(y, x, ch);
		}
	when S_GFIND:
	    /*
	     * Potion of gold detection
	     */
	    ch = FALSE;
	    wclear(hw);
	    for (op = lvl_obj; op != NULL; op = next(op))
		if (op->o_type == GOLD)
		{
		    ch = TRUE;
		    mvwaddrawch(hw, op->o_pos.y, op->o_pos.x, GOLD);
		}
	    if (ch)
	    {
		s_know[S_GFIND] = TRUE;
		show_win(hw,
		    "You begin to feel greedy and you sense gold.--More--");
	    }
	    else
		msg("you feel a pull downward");
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make him dissapear and reappear
	     */
	    {
		register struct room *cur_room;

		cur_room = proom;
		teleport();
		if (cur_room != proom)
		    s_know[S_TELEP] = TRUE;
	    }
	when S_ENCH:
	    if (cur_weapon == NULL || cur_weapon->o_type != WEAPON)
		msg("you feel a strange sense of loss");
	    else
	    {
		cur_weapon->o_flags &= ~ISCURSED;
		if (rnd(2) == 0)
		    cur_weapon->o_hplus++;
		else
		    cur_weapon->o_dplus++;
		msg("your %s glows %s for a moment",
		    w_names[cur_weapon->o_which],
		    on(player, ISTrip) ? rnd_color() : "blue");
	    }
	when S_SCARE:
	    /*
	     * Reading it is a mistake and produces laughter at the
	     * poor rogue's boo boo.
	     */
	    msg("you hear maniacal laughter in the distance");
	when S_REMOVE:
	    if (cur_armor != NULL)
		cur_armor->o_flags &= ~ISCURSED;
	    if (cur_weapon != NULL)
		cur_weapon->o_flags &= ~ISCURSED;
	    if (cur_ring[LEFT] != NULL)
		cur_ring[LEFT]->o_flags &= ~ISCURSED;
	    if (cur_ring[RIGHT] != NULL)
		cur_ring[RIGHT]->o_flags &= ~ISCURSED;
	    if (on(player, ISTrip))
		msg("you feel in touch with the Universal Onenes");
	    else
		msg("you feel as if somebody is watching over you");
	when S_AGGR:
	    /*
	     * This scroll aggravates all the monsters on the current
	     * level and sets them running towards the hero
	     */
	    aggravate();
	    msg("you hear a high pitched humming noise");
	when S_NOP:
	    if (on(player, ISTrip))
		msg("what an interesting piece of paper.  Like, groovy, man!");
	    else
		msg("this scroll seems to be blank");
	when S_GENOCIDE:
	    s_know[S_GENOCIDE] = TRUE;
	    msg("you have been granted the boon of genocide");
	    genocide();
	otherwise:
	    msg("what a puzzling scroll!");
	    return;
    }
    look(TRUE);	/* put the result of the scroll on the screen */
    status();
    /*
     * Get rid of the thing
     */
    inpack--;
    if (obj->o_count > 1)
	obj->o_count--;
    else
    {
	detach(pack, obj);
	discardit = TRUE;
    }

    call_it(s_know[obj->o_which], &s_guess[obj->o_which]);

    if (discardit)
	discard(obj);
    del_obj = NULL;
}
