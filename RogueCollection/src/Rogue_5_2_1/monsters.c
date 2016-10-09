/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	4.24 (Berkeley) 4/6/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <string.h>
#include <ctype.h>
#include "rogue.h"

/*
 * List of monsters in rough order of vorpalness
 *
 * NOTE: This not initialized using strings so that xstr doesn't set up
 * the string not to be saved.  Otherwise genocide is lost through
 * saving a game.
 */
char lvl_mons[] =  {
    'K', 'J', 'B', 'S', 'H', 'E', 'A', 'O', 'Z', 'G', 'L', 'C', 'R',
    'Q', 'N', 'Y', 'T', 'W', 'F', 'I', 'X', 'U', 'M', 'V', 'P', 'D',
    '\0'
};

char wand_mons[] = {
    'K', 'J', 'B', 'S', 'H', ' ', 'A', 'O', 'Z', 'G', ' ', 'C', 'R',
    'Q', ' ', 'Y', 'T', 'W', ' ', 'I', 'X', 'U', ' ', 'V', 'P', ' ',
    '\0'
};

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
new_monster(tp, type, cp)
register THING *tp;
char type;
register coord *cp;
{
    register struct monster *mp;
    register int lev_add;

    if ((lev_add = level - AMULETLEVEL) < 0)
	lev_add = 0;
    attach(mlist, tp);
    tp->t_type = type;
    tp->t_disguise = type;
    tp->t_pos = *cp;
    tp->t_oldch = mvinch(cp->y, cp->x);
    tp->t_room = roomin(cp);
    moat(cp->y, cp->x) = tp;
    mp = &monsters[tp->t_type-'A'];
    tp->t_stats.s_lvl = mp->m_stats.s_lvl + lev_add;
    tp->t_stats.s_maxhp = tp->t_stats.s_hpt = roll(tp->t_stats.s_lvl, 8);
    tp->t_stats.s_arm = mp->m_stats.s_arm - lev_add;
    strncpy(tp->t_stats.s_dmg,mp->m_stats.s_dmg,16);
    tp->t_stats.s_str = mp->m_stats.s_str;
    tp->t_stats.s_exp = mp->m_stats.s_exp + lev_add * 10 + exp_add(tp);
    tp->t_flags = mp->m_flags;
    tp->t_turn = TRUE;
    tp->t_pack = NULL;
    if (ISWEARING(R_AGGR))
	runto(cp, &hero);
    if (type == 'M')
	switch (rnd(level > 25 ? 9 : 8))
	{
	    case 0: tp->t_disguise = GOLD;
	    when 1: tp->t_disguise = POTION;
	    when 2: tp->t_disguise = SCROLL;
	    when 3: tp->t_disguise = STAIRS;
	    when 4: tp->t_disguise = WEAPON;
	    when 5: tp->t_disguise = ARMOR;
	    when 6: tp->t_disguise = RING;
	    when 7: tp->t_disguise = STICK;
	    when 8: tp->t_disguise = AMULET;
	}
}

/*
 * expadd:
 *	Experience to add for this monster's level/hit points
 */
exp_add(tp)
register THING *tp;
{
    register int mod;

    if (tp->t_stats.s_lvl == 1)
	mod = tp->t_stats.s_maxhp / 8;
    else
	mod = tp->t_stats.s_maxhp / 6;
    if (tp->t_stats.s_lvl > 9)
	mod *= 20;
    else if (tp->t_stats.s_lvl > 6)
	mod *= 4;
    return mod;
}

/*
 * wanderer:
 *	Create a new wandering monster and aim it at the player
 */
wanderer()
{
    register int i;
    register struct room *rp;
    register THING *tp;
    coord cp = {0,0};
    register int cnt = 0;

    tp = new_item();
    do
    {
        /* Avoid endless loop when all rooms are filled with monsters
	 * and the player room is not accessible to the monsters.
	 */
	if (cnt++ >= 500)
	{
	    discard(tp);
	    return;
	}
	i = rnd_room();
	if ((rp = &rooms[i]) == proom)
	    continue;
	rnd_pos(rp, &cp);
    } until (rp != proom && step_ok(winat(cp.y, cp.x)));
    new_monster(tp, randmonster(TRUE), &cp);
    runto(&tp->t_pos, &hero);
#ifdef WIZARD
    if (wizard)
	msg("started a wandering %s", monsters[tp->t_type-'A'].m_name);
#endif
}

/*
 * wake_monster:
 *	What to do when the hero steps next to a monster
 */
THING *
wake_monster(y, x)
int y, x;
{
    register THING *tp;
    register struct room *rp;
    register char ch;

#ifdef WIZARD
    if ((tp = moat(y, x)) == NULL)
	msg("can't find monster in wake_monster");
#else
    tp = moat(y, x);
#endif
    ch = tp->t_type;
    /*
     * Every time he sees mean monster, it might start chasing him
     */
    if (!on(*tp, ISRUN) && rnd(3) != 0 && on(*tp, ISMEAN) && !on(*tp, ISHELD)
	&& !ISWEARING(R_STEALTH))
    {
	tp->t_dest = &hero;
	tp->t_flags |= ISRUN;
    }
    if (ch == 'U' && !on(player, ISBLIND) && !on(*tp, ISFOUND)
	&& !on(*tp, ISCANC) && on(*tp, ISRUN))
    {
        rp = proom;
	if ((rp != NULL && !(rp->r_flags & ISDARK))
	    || DISTANCE(y, x, hero.y, hero.x) < LAMPDIST)
	{
	    tp->t_flags |= ISFOUND;
	    if (!save(VS_MAGIC))
	    {
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(20) + HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(20) + HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
		msg("the umber hulk's gaze has confused you");
	    }
	}
    }
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && !on(*tp, ISRUN))
    {
	tp->t_flags |= ISRUN;
	if (proom->r_goldval)
	    tp->t_dest = &proom->r_gold;
	else
	    tp->t_dest = &hero;
    }
    return tp;
}

/*
 * genocide:
 *	Wipe one monster out of existence (for now...)
 */
genocide()
{
    register THING *mp;
    register char c;
    register int i;
    register THING *nmp;

    addmsg("which monster");
    if (!terse)
	addmsg(" do you wish to wipe out");
    msg("? ");
    while (!isalpha(c = readchar()))
	if (c == ESCAPE)
	    return;
	else
	{
	    mpos = 0;
	    msg("please specifiy a letter between 'A' and 'Z'");
	}
    mpos = 0;
    if (islower(c))
	c = toupper(c);
    for (mp = mlist; mp; mp = nmp)
    {
	nmp = next(mp);
	if (mp->t_type == c)
	    remove_monster(&mp->t_pos, mp, FALSE);
    }
    for (i = 0; i < 26; i++)
	if (lvl_mons[i] == c)
	{
	    lvl_mons[i] = ' ';
	    wand_mons[i] = ' ';
	    break;
	}
    if (!terse)
	addmsg("there will be ");
    msg("no more %ss", monsters[c - 'A'].m_name);
}

/*
 * give_pack:
 *	Give a pack to a monster if it deserves one
 */
give_pack(tp)
register THING *tp;
{
    if (rnd(100) < monsters[tp->t_type-'A'].m_carry)
	attach(tp->t_pack, new_thing());
}
