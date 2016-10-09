/*
 * Read a scroll and let it happen
 *
 * scrolls.c	1.4 (AI Design)	12/14/84
 */

#include "rogue.h"
#include "curses.h"

char *laugh = "you hear maniacal laughter%s.";
char *in_dist = " in the distance";
/*
 * read_scroll:
 *	Read a scroll from the pack and do the appropriate thing
 */
read_scroll()
{
    register THING *obj;
    register int y, x;
    register byte ch;
    register THING *op;
    register int index;
    register bool discardit = FALSE;

    obj = get_item("read", SCROLL);
    if (obj == NULL)
		return;
    if (obj->o_type != SCROLL){
		msg("there is nothing on it to read");
		return;
    }
    ifterse0("the scroll vanishes","as you read the scroll, it vanishes");
    /*
     * Calculate the effect it has on the poor guy.
     */
    if (obj == cur_weapon)
		cur_weapon = NULL;
    switch (obj->o_which){
	when S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    player.t_flags |= CANHUH;
	    msg("your hands begin to glow red");
	when S_ARMOR:
	    if (cur_armor != NULL) {
			cur_armor->o_ac--;
			cur_armor->o_flags &= ~ISCURSED;
			ifterse0("your armor glows faintly","your armor glows faintly for a moment");
	    }
	when S_HOLD:
	    /*
	     * Hold monster scroll.  Stop all monsters within two spaces
	     * from chasing after the hero.
	     */

	    for (x = hero.x - 3; x <= hero.x + 3; x++)
			if (x >= 0 && x < COLS)
			    for (y = hero.y - 3; y <= hero.y + 3; y++)
					if ((y > 0 && y < maxrow) && ((op=moat(y, x)) != NULL)) {
					    op->t_flags &= ~ISRUN;
					    op->t_flags |= ISHELD;
					}
	when S_SLEEP:
	    /*
	     * Scroll which makes you fall asleep
	     */
	    s_know[S_SLEEP] = TRUE;
	    no_command += rnd(SLEEPTIME) + 4;
	    player.t_flags &= ~ISRUN;
	    msg("you fall asleep");
	when S_CREATE:
	    {
		coord mp;

		if (plop_monster(hero.y, hero.x, &mp) && (op=new_item()) != NULL)
			new_monster(op, randmonster(FALSE), &mp);
		else
			ifterse0("you hear a faint cry of anguish",
				"you hear a faint cry of anguish in the distance");
	    }
	when S_IDENT:
	    /*
	     * Identify, let the rogue figure something out
	     */
	    s_know[S_IDENT] = TRUE;
	    msg("this scroll is an identify scroll");
	    if (! strcmp(s_menu,"on") || !strcmp(s_menu,"sel"))
	    	more(" More ");
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
	    for (y = 1; y < maxrow; y++)
			for (x = 0; x < COLS; x++) {
			    index = INDEX(y, x);
			    switch (ch = _level[index])
			    {
				case VWALL:
				case HWALL:
				case ULWALL:
				case URWALL:
				case LLWALL:
				case LRWALL:
				    if (!(_flags[index] & F_REAL)) {
						ch = _level[index] = DOOR;
						_flags[index] &= ~F_REAL;
				    }
				case DOOR:
				case PASSAGE:
				case STAIRS:
				    if ((op = moat(y, x)) != NULL)
						if (op->t_oldch == ' ')
						    op->t_oldch = ch;
				    break;
				default:
				    ch = ' ';
			    }
			    if (ch == DOOR) {
			        move(y,x);
			        if (inch() != DOOR)
			            standout();
			    }
			    if (ch != ' ')
					mvaddch(y, x, ch);
			    standend();
			}
	when S_GFIND:
	    /*
	     * Scroll of food detection
	     */
	    ch = FALSE;
	    for (op = lvl_obj; op != NULL; op = next(op)) {
			if (op->o_type == FOOD) {
                ch = TRUE;
			    standout();
			    mvwaddch(hw, op->o_pos.y, op->o_pos.x, FOOD);
			    standend();
			} else /* as a bonus this will detect amulets as well */
			if (op->o_type == AMULET) {
                ch = TRUE;
			    standout();
			    mvwaddch(hw, op->o_pos.y, op->o_pos.x, AMULET);
			    standend();
			}
	    }
	    if (ch) {
			s_know[S_GFIND] = TRUE;
			msg("your nose tingles as you sense food");
	    } else
			ifterse0("you hear a growling noise close by","you hear a growling noise very close to you");
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
		ifterse1("your %s glows blue","your %s glows blue for a moment", w_names[cur_weapon->o_which]);
	    }
	when S_SCARE:
	    /*
	     * Reading it is a mistake and produces laughter at the
	     * poor rogue's boo boo.
	     */
    	    msg(laugh, terse || expert ? "" : in_dist);
	when S_REMOVE:
	    if (cur_armor != NULL)
			cur_armor->o_flags &= ~ISCURSED;
	    if (cur_weapon != NULL)
			cur_weapon->o_flags &= ~ISCURSED;
	    if (cur_ring[LEFT] != NULL)
			cur_ring[LEFT]->o_flags &= ~ISCURSED;
	    if (cur_ring[RIGHT] != NULL)
			cur_ring[RIGHT]->o_flags &= ~ISCURSED;
	    ifterse0("somebody is watching over you","you feel as if somebody is watching over you");
	when S_AGGR:
	    /*
	     * This scroll aggravates all the monsters on the current
	     * level and sets them running towards the hero
	     */
	    aggravate();
	    ifterse("you hear a humming noise",
					"you hear a high pitched humming noise");
	when S_NOP:
	    msg("this scroll seems to be blank");
	when S_VORPAL:
	    /*
	     * Extra Vorpal Enchant Weapon
	     *     Give weapon +1,+1
	     *     Is extremely vorpal against one certain type of monster
	     *     Against this type (o_enemy) the weapon gets:
	     *		+4,+4
	     *		The ability to zap one such monster into oblivion
	     *
	     *     Some of these are cursed and if the rogue misses her saving
	     *     throw she will be forced to attack monsters of this type
	     *     whenever she sees one (not yet implemented)
	     *
	     * If he doesn't have a weapon I get to chortle again!
	     */
	    if (cur_weapon == NULL || cur_weapon->o_type != WEAPON)
    		msg(laugh, terse || expert ? "" : in_dist);
	    else {
			/*
			 * You aren't allowed to doubly vorpalize a weapon.
			 */
			if (cur_weapon->o_enemy != 0) {
			    msg("your %s vanishes in a puff of smoke",
				w_names[cur_weapon->o_which]);
			    detach(pack, cur_weapon);
			    discard(cur_weapon);
			    cur_weapon = NULL;
			} else {
			    cur_weapon->o_enemy = pick_mons();
			    cur_weapon->o_hplus++;
			    cur_weapon->o_dplus++;
			    cur_weapon->o_charges = 1;
			    msg(flash, w_names[cur_weapon->o_which],
		    		terse || expert ? "" : intense);

			    /*
			     * Sometimes this is a mixed blessing ...
				    if (rnd(20) == 0) {
						cur_weapon->o_flags |= ISCURSED;
						if (!save(VS_MAGIC)) {
						    cur_weapon->o_flags |= ISEGO|ISREVEAL;
						    s_know[S_VORPAL] = TRUE;
						    msg("you feel a sudden desire to kill %ss.",
							monsters[cur_weapon->o_enemy-'A'].m_name);
						}
				    }
			     */
			}
	    }
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
}
