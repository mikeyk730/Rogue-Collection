/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	3.18 (Berkeley) 6/15/81
 */

#include "curses.h"
#include "rogue.h"
#include <ctype.h>

/*
 * List of monsters in rough order of vorpalness
 */
char lvl_mons[27] =  "KJBSHEAOZGLCRQNYTWFIXUMVDP";
char wand_mons[27] = "KJBSH AOZG CRQ Y W IXU V  ";

/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */

randmonster(wander)
bool wander;
{
    register int d;
    register char *mons;

    mons = wander ? wand_mons : lvl_mons;
    do
    {
	d = level + (rnd(10) - 5);
	if (d < 1)
	    d = rnd(5) + 1;
	if (d > 26)
	    d = rnd(5) + 22;
    } while (mons[--d] == ' ');
    return mons[d];
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */

new_monster(item, type, cp)
struct linked_list *item;
char type;
register coord *cp;
{
    register struct thing *tp;
    register struct monster *mp;

    attach(mlist, item);
    tp = (struct thing *) ldata(item);
    tp->t_type = type;
    tp->t_pos = *cp;
    tp->t_oldch = mvwinch(cw, cp->y, cp->x);
    mvwaddch(mw, cp->y, cp->x, tp->t_type);
    mp = &monsters[tp->t_type-'A'];
    tp->t_stats.s_hpt = roll(mp->m_stats.s_lvl, 8);
    tp->t_stats.s_lvl = mp->m_stats.s_lvl;
    tp->t_stats.s_arm = mp->m_stats.s_arm;
    strcpy(tp->t_stats.s_dmg,mp->m_stats.s_dmg);
    tp->t_stats.s_exp = mp->m_stats.s_exp;
    tp->t_stats.s_str.st_str = 10;
    tp->t_flags = mp->m_flags;
    tp->t_turn = TRUE;
    tp->t_pack = NULL;
    if (ISWEARING(R_AGGR))
	runto(cp, &hero);
    if (type == 'M')
    {
	char mch;

	if (tp->t_pack != NULL)
	    mch = ((struct object *) ldata(tp->t_pack))->o_type;
	else
	    switch (rnd(level > 25 ? 9 : 8))
	    {
		case 0: mch = GOLD;
		when 1: mch = POTION;
		when 2: mch = SCROLL;
		when 3: mch = STAIRS;
		when 4: mch = WEAPON;
		when 5: mch = ARMOR;
		when 6: mch = RING;
		when 7: mch = STICK;
		when 8: mch = AMULET;
	    }
	tp->t_disguise = mch;
    }
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */

wanderer()
{
    register int i, ch;
    register struct room *rp, *hr = roomin(&hero);
    register struct linked_list *item;
    register struct thing *tp;
    coord cp;

    item = new_item(sizeof *tp);
    do
    {
	i = rnd_room();
	if ((rp = &rooms[i]) == hr)
	    continue;
	rnd_pos(rp, &cp);
	if ((ch = mvwinch(stdscr, cp.y, cp.x)) == ERR)
	{
	    debug("Routine wanderer: mvwinch failed to %d,%d", cp.y, cp.x);
	    wait_for('\n');
	    return;
	}
    } until(hr != rp && step_ok(ch));
    new_monster(item, randmonster(TRUE), &cp);
    tp = (struct thing *) ldata(item);
    tp->t_flags |= ISRUN;
    tp->t_pos = cp;
    tp->t_dest = &hero;
    if (wizard)
	msg("Started a wandering %s", monsters[tp->t_type-'A'].m_name);
}

/*
 * what to do when the hero steps next to a monster
 */
struct linked_list *
wake_monster(y, x)
int y, x;
{
    register struct thing *tp;
    register struct linked_list *it;
    register struct room *rp;
    register char ch;

    if ((it = find_mons(y, x)) == NULL)
	msg("Can't find monster in show");
    tp = (struct thing *) ldata(it);
    ch = tp->t_type;
    /*
     * Every time he sees mean monster, it might start chasing him
     */
    if (rnd(100) > 33 && on(*tp, ISMEAN) && off(*tp, ISHELD)
	&& !ISWEARING(R_STEALTH))
    {
	tp->t_dest = &hero;
	tp->t_flags |= ISRUN;
    }
    if (ch == 'U' && off(player, ISBLIND))
    {
        rp = roomin(&hero);
	if ((rp != NULL && !(rp->r_flags&ISDARK))
	    || DISTANCE(y, x, hero.y, hero.x) < 3)
	{
	    if (off(*tp, ISFOUND) && !save(VS_MAGIC))
	    {
		msg("The umber hulk's gaze has confused you.");
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(20)+HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(20)+HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
	    }
	    tp->t_flags |= ISFOUND;
	}
    }
    /*
     * Hide invisible monsters
     */
    if (on(*tp, ISINVIS) && off(player, CANSEE))
	ch = mvwinch(stdscr, y, x);
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && off(*tp, ISRUN))
	if (rp != NULL && rp->r_goldval)
	{
	    tp->t_dest = &rp->r_gold;
	    tp->t_flags |= ISRUN;
	}
    return it;
}

genocide()
{
    register struct linked_list *ip;
    register struct thing *mp;
    register char c;
    register int i;
    register struct linked_list *nip;

    addmsg("Which monster");
    if (!terse)
	addmsg(" do you wish to wipe out");
    msg("? ");
    while (!isalpha(c = readchar()))
	if (c == ESCAPE)
	    return;
	else
	{
	    mpos = 0;
	    msg("Please specifiy a letter between 'A' and 'Z'");
	}
    if (islower(c))
	c = toupper(c);
    for (ip = mlist; ip; ip = nip)
    {
	mp = (struct thing *) ldata(ip);
	nip = next(ip);
	if (mp->t_type == c)
	    remove_monster(&mp->t_pos, ip);
    }
    for (i = 0; i < 26; i++)
	if (lvl_mons[i] == c)
	{
	    lvl_mons[i] = ' ';
	    wand_mons[i] = ' ';
	    break;
	}
}
