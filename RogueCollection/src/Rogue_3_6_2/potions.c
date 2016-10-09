/*
 * 	@(#)potions.c	3.1	3.1	5/7/81
 * Function(s) for dealing with potions
 */

#include "curses.h"
#include <stdlib.h>
#include "rogue.h"

quaff()
{
    register struct object *obj;
    register struct linked_list *item, *titem;
    register struct thing *th;
    char buf[80];

    item = get_item("quaff", POTION);
    /*
     * Make certain that it is somethings that we want to drink
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != POTION)
    {
	if (!terse)
	    msg("Yuk! Why would you want to drink that?");
	else
	    msg("That's undrinkable");
	return;
    }
    if (obj == cur_weapon)
	cur_weapon = NULL;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch(obj->o_which)
    {
	case P_CONFUSE:
	    if (off(player, ISHUH))
	    {
		msg("Wait, what's going on here. Huh? What? Who?");
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8)+HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8)+HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
	    }
	    p_know[P_CONFUSE] = TRUE;
	when P_POISON:
	    if (!ISWEARING(R_SUSTSTR))
	    {
		chg_str(-(rnd(3)+1));
		msg("You feel very sick now.");
	    }
	    else
		msg("You feel momentarily sick");
	    p_know[P_POISON] = TRUE;
	when P_HEALING:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg("You begin to feel better.");
	    sight();
	    p_know[P_HEALING] = TRUE;
	when P_STRENGTH:
	    msg("You feel stronger, now.  What bulging muscles!");
	    chg_str(1);
	    p_know[P_STRENGTH] = TRUE;
	when P_MFIND:
	    /*
	     * Potion of monster detection, if there are monters, detect them
	     */
	    if (mlist != NULL)
	    {
		wclear(hw);
		overwrite(mw, hw);
		show_win(hw,
		    "You begin to sense the presence of monsters.--More--");
		p_know[P_MFIND] = TRUE;
	    }
	    else
		msg("You have a strange feeling for a moment, then it passes.");
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    if (lvl_obj != NULL)
	    {
		struct linked_list *mobj;
		struct object *tp;
		bool show;

		show = FALSE;
		wclear(hw);
		for (mobj = lvl_obj; mobj != NULL; mobj = next(mobj))
		{
		    tp = (struct object *) ldata(mobj);
		    if (is_magic(tp))
		    {
			show = TRUE;
			mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, MAGIC);
		    }
		    p_know[P_TFIND] = TRUE;
		}
		for (titem = mlist; titem != NULL; titem = next(titem))
		{
		    register struct linked_list *pitem;

		    th = (struct thing *) ldata(titem);
		    for (pitem = th->t_pack; pitem != NULL; pitem = next(pitem))
		    {
			if (is_magic(ldata(pitem)))
			{
			    show = TRUE;
			    mvwaddch(hw, th->t_pos.y, th->t_pos.x, MAGIC);
			}
			p_know[P_TFIND] = TRUE;
		    }
		}
		if (show)
		{
		    show_win(hw, 
			"You sense the presence of magic on this level.--More--");
		    break;
		}
	    }
	    msg("You have a strange feeling for a moment, then it passes.");
	when P_PARALYZE:
	    msg("You can't move.");
	    no_command = HOLDTIME;
	    p_know[P_PARALYZE] = TRUE;
	when P_SEEINVIS:
	    msg("This potion tastes like %s juice.", fruit);
	    if (off(player, CANSEE))
	    {
		player.t_flags |= CANSEE;
		fuse(unsee, 0, SEEDURATION, AFTER);
		light(&hero);
	    }
	    sight();
	when P_RAISE:
	    msg("You suddenly feel much more skillful");
	    p_know[P_RAISE] = TRUE;
	    raise_level();
	when P_XHEAL:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg("You begin to feel much better.");
	    p_know[P_XHEAL] = TRUE;
	    sight();
	when P_HASTE:
	    add_haste(TRUE);
	    msg("You feel yourself moving much faster.");
	    p_know[P_HASTE] = TRUE;
	when P_RESTORE:
	    msg("Hey, this tastes great.  It make you feel warm all over.");
	    if (pstats.s_str.st_str < max_stats.s_str.st_str ||
		(pstats.s_str.st_str == 18 &&
		 pstats.s_str.st_add < max_stats.s_str.st_add))
	    pstats.s_str = max_stats.s_str;
	when P_BLIND:
	    msg("A cloak of darkness falls around you.");
	    if (off(player, ISBLIND))
	    {
		player.t_flags |= ISBLIND;
		fuse(sight, 0, SEEDURATION, AFTER);
		look(FALSE);
	    }
	    p_know[P_BLIND] = TRUE;
	when P_NOP:
	    msg("This potion tastes extremely dull.");
	otherwise:
	    msg("What an odd tasting potion!");
	    return;
    }
    status();
    if (p_know[obj->o_which] && p_guess[obj->o_which])
    {
	free(p_guess[obj->o_which]);
	p_guess[obj->o_which] = NULL;
    }
    else if (!p_know[obj->o_which] && askme && p_guess[obj->o_which] == NULL)
    {
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (get_str(buf, cw) == NORM)
	{
	    p_guess[obj->o_which] = malloc((unsigned int) strlen(buf) + 1);
	    strcpy(p_guess[obj->o_which], buf);
	}
    }
    /*
     * Throw the item away
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
