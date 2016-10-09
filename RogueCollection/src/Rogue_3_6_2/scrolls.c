
/*
 * Read a scroll and let it happen
 *
 * @(#)scrolls.c	3.5 (Berkeley) 6/15/81
 */

#include "curses.h"
#include <stdlib.h>
#include <ctype.h>
#include "rogue.h"

read_scroll()
{
    register struct object *obj;
    register struct linked_list *item;
    register struct room *rp;
    register int i,j;
    register char ch, nch;
    register struct linked_list *titem;
    char buf[80];

    item = get_item("read", SCROLL);
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != SCROLL)
    {
	if (!terse)
	    msg("There is nothing on it to read");
	else
	    msg("Nothing to read");
	return;
    }
    msg("As you read the scroll, it vanishes.");
    /*
     * Calculate the effect it has on the poor guy.
     */
    if (obj == cur_weapon)
	cur_weapon = NULL;
    switch(obj->o_which)
    {
	case S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    msg("Your hands begin to glow red");
	    player.t_flags |= CANHUH;
	when S_LIGHT:
	    s_know[S_LIGHT] = TRUE;
	    if ((rp = roomin(&hero)) == NULL)
		msg("The corridor glows and then fades");
	    else
	    {
		addmsg("The room is lit");
		if (!terse)
		    addmsg(" by a shimmering blue light.");
		endmsg();
		rp->r_flags &= ~ISDARK;
		/*
		 * Light the room and put the player back up
		 */
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    }
	when S_ARMOR:
	    if (cur_armor != NULL)
	    {
		msg("Your armor glows faintly for a moment");
		cur_armor->o_ac--;
		cur_armor->o_flags &= ~ISCURSED;
	    }
	when S_HOLD:
	    /*
	     * Hold monster scroll.  Stop all monsters within two spaces
	     * from chasing after the hero.
	     */
	    {
		register int x,y;
		register struct linked_list *mon;

		for (x = hero.x-2; x <= hero.x+2; x++)
		    for (y = hero.y-2; y <= hero.y+2; y++)
			if (y > 0 && x > 0 && isupper(mvwinch(mw, y, x)))
			    if ((mon = find_mons(y, x)) != NULL)
			    {
				register struct thing *th;

				th = (struct thing *) ldata(mon);
				th->t_flags &= ~ISRUN;
				th->t_flags |= ISHELD;
			    }
	    }
	when S_SLEEP:
	    /*
	     * Scroll which makes you fall asleep
	     */
	    s_know[S_SLEEP] = TRUE;
	    msg("You fall asleep.");
	    no_command += 4 + rnd(SLEEPTIME);
	when S_CREATE:
	    /*
	     * Create a monster
	     * First look in a circle around him, next try his room
	     * otherwise give up
	     */
	    {
		register int x, y;
		register bool appear = 0;
		coord mp;

		/*
		 * Search for an open place
		 */
		for (y = hero.y; y <= hero.y+1; y++)
		    for (x = hero.x; x <= hero.x+1; x++)
		    {
			/*
			 * Don't put a monster in top of the player.
			 */
			if (y == hero.y && x == hero.x)
			    continue;
			/*
			 * Or anything else nasty
			 */
			if (step_ok(winat(y, x)))
			{
			    if (rnd(++appear) == 0)
			    {
				mp.y = y;
				mp.x = x;
			    }
			}
		    }
		if (appear)
		{
		    titem = new_item(sizeof (struct thing));
		    new_monster(titem, randmonster(FALSE), &mp);
		}
		else
		    msg("You hear a faint cry of anguish in the distance.");
	    }
	when S_IDENT:
	    /*
	     * Identify, let the rogue figure something out
	     */
	    msg("This scroll is an identify scroll");
	    s_know[S_IDENT] = TRUE;
	    whatis();
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    s_know[S_MAP] = TRUE;
	    msg("Oh, now this scroll has a map on it.");
	    overwrite(stdscr, hw);
	    /*
	     * Take all the things we want to keep hidden out of the window
	     */
	    for (i = 0; i < LINES; i++)
		for (j = 0; j < COLS; j++)
		{
		    switch (nch = ch = mvwinch(hw, i, j))
		    {
			case SECRETDOOR:
                            nch = DOOR;
			    mvaddch(i, j, nch);
			case '-':
			case '|':
			case DOOR:
			case PASSAGE:
			case ' ':
			case STAIRS:
			    if (mvwinch(mw, i, j) != ' ')
			    {
				register struct thing *it;

				it = (struct thing *) ldata(find_mons(i, j));
				if (it->t_oldch == ' ')
				    it->t_oldch = nch;
			    }
			    break;
			default:
			    nch = ' ';
		    }
		    if (nch != ch)
			waddch(hw, nch);
		}
	    /*
	     * Copy in what he has discovered
	     */
	    overlay(cw, hw);
	    /*
	     * And set up for display
	     */
	    overwrite(hw, cw);
	when S_GFIND:
	    /*
	     * Potion of gold detection
	     */
	    {
		int gtotal = 0;

		wclear(hw);
		for (i = 0; i < MAXROOMS; i++)
		{
		    gtotal += rooms[i].r_goldval;
		    if (rooms[i].r_goldval != 0 &&
			mvwinch(stdscr, rooms[i].r_gold.y, rooms[i].r_gold.x)
			== GOLD)
			mvwaddch(hw,rooms[i].r_gold.y,rooms[i].r_gold.x,GOLD);
		}
		if (gtotal)
		{
		    s_know[S_GFIND] = TRUE;
		    show_win(hw,
			"You begin to feel greedy and you sense gold.--More--");
		}
		else msg("You begin to feel a pull downward");
	    }
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make him dissapear and reappear
	     */
	    {
		int rm;
		struct room *cur_room;

		cur_room = roomin(&hero);
		rm = teleport();
		if (cur_room != &rooms[rm])
		    s_know[S_TELEP] = TRUE;
	    }
	when S_ENCH:
	    if (cur_weapon == NULL)
		msg("You feel a strange sense of loss.");
	    else
	    {
		cur_weapon->o_flags &= ~ISCURSED;
		if (rnd(100) > 50)
		    cur_weapon->o_hplus++;
		else
		    cur_weapon->o_dplus++;
		msg("Your %s glows blue for a moment.", w_names[cur_weapon->o_which]);
	    }
	when S_SCARE:
	    /*
	     * A monster will refuse to step on a scare monster scroll
	     * if it is dropped.  Thus reading it is a mistake and produces
	     * laughter at the poor rogue's boo boo.
	     */
	    msg("You hear maniacal laughter in the distance.");
	when S_REMOVE:
	    if (cur_armor != NULL)
		cur_armor->o_flags &= ~ISCURSED;
	    if (cur_weapon != NULL)
		cur_weapon->o_flags &= ~ISCURSED;
	    if (cur_ring[LEFT] != NULL)
		cur_ring[LEFT]->o_flags &= ~ISCURSED;
	    if (cur_ring[RIGHT] != NULL)
		cur_ring[RIGHT]->o_flags &= ~ISCURSED;
	    msg("You feel as if somebody is watching over you.");
	when S_AGGR:
	    /*
	     * This scroll aggravates all the monsters on the current
	     * level and sets them running towards the hero
	     */
	    aggravate();
	    msg("You hear a high pitched humming noise.");
	when S_NOP:
	    msg("This scroll seems to be blank.");
	when S_GENOCIDE:
	    msg("You have been granted the boon of genocide");
	    genocide();
	    s_know[S_GENOCIDE] = TRUE;
	otherwise:
	    msg("What a puzzling scroll!");
	    return;
    }
    look(TRUE);	/* put the result of the scroll on the screen */
    status();
    if (s_know[obj->o_which] && s_guess[obj->o_which])
    {
	free(s_guess[obj->o_which]);
	s_guess[obj->o_which] = NULL;
    }
    else if (!s_know[obj->o_which] && askme && s_guess[obj->o_which] == NULL)
    {
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (get_str(buf, cw) == NORM)
	{
	    s_guess[obj->o_which] = malloc((unsigned int) strlen(buf) + 1);
	    strcpy(s_guess[obj->o_which], buf);
	}
    }
    /*
     * Get rid of the thing
     */
    inpack--;
    if (obj->o_count > 1)
	obj->o_count--;
    else
    {
	detach(pack, item);
        discard(item);
    }
}
