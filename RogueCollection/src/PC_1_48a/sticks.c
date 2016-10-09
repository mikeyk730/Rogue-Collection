/*
 * Functions to implement the various sticks one might find
 * while wandering around the dungeon.
 *
 * @(#)sticks.c		1.2 (AI Design)		2/12/84
 */

#include "rogue.h"
#include "curses.h"

/*
 * fix_stick:
 *	Set up a new stick
 */
fix_stick(cur)
register THING *cur;
{
    if (strcmp(ws_type[cur->o_which], "staff") == 0)
	cur->o_damage = "2d3";
    else
	cur->o_damage = "1d1";
    cur->o_hurldmg = "1d1";

    cur->o_charges = 3 + rnd(5);
    switch (cur->o_which)
    {
	when WS_HIT:
	    cur->o_hplus = 100;
	    cur->o_dplus = 3;
	    cur->o_damage = "1d8";
	when WS_LIGHT:
	    cur->o_charges = 10 + rnd(10);
    }
}

/*
 * do_zap:
 *	Perform a zap with a wand
 */
do_zap()
{
    THING *obj;
    THING *tp;
    register int y, x;
    register char *name;
    int which_one;

    if ((obj = get_item("zap with", STICK)) == NULL)
	    return;
    which_one = obj->o_which;
    if (obj->o_type != STICK)
    {
	if (obj->o_enemy && obj->o_charges)
	    which_one = MAXSTICKS;
	else
	{
	    msg("you can't zap with that!");
	    after = FALSE;
	    return;
	}
    }
    if (obj->o_charges == 0)
    {
	msg("nothing happens");
	return;
    }
    switch (which_one)
    {
	when WS_LIGHT:
	    /*
	     * Reddy Kilowat wand.  Light up the room
	     */
	    if (on(player,ISBLIND))
	    	msg("you feel a warm glow around you");
	    else 
	    {
		ws_know[WS_LIGHT] = TRUE;
	        if (proom->r_flags & ISGONE)
		    msg("the corridor glows and then fades");
	        else
		    msg("the room is lit by a shimmering blue light");
	    }
	    if (!(proom->r_flags & ISGONE))
	    {
		proom->r_flags &= ~ISDARK;
		/*
		 * Light the room and put the player back up
		 */
		enter_room(&hero);
	    }
	when WS_DRAIN:
	    /*
	     * Take away 1/2 of hero's hit points, then take it away
	     * evenly from the monsters in the room (or next to hero
	     * if he is in a passage)
	     */
	    if (pstats.s_hpt < 2)
	    {
		msg("you are too weak to use it");
		return;
	    }
	    else
		drain();
	when WS_POLYMORPH:
	case WS_TELAWAY:
	case WS_TELTO:
	case WS_CANCEL:
	case MAXSTICKS:			/* Special case for vorpal weapon */
	{
	    register byte monster, oldch;
	    register int rm;
	    coord new_yx;

	    y = hero.y;
	    x = hero.x;
	    while (step_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ((tp = moat(y, x)) != NULL)
	    {
		register byte omonst;

		omonst = monster = tp->t_type;
		if (monster == 'F')
		    player.t_flags &= ~ISHELD;
		if (which_one == MAXSTICKS)
		{
		    if (monster == obj->o_enemy)
		    {
			msg("the %s vanishes in a puff of smoke",
			    monsters[monster-'A'].m_name);
			killed(tp, FALSE);
		    }
		    else
			msg("you hear a maniacal chuckle in the distance.");
		}
		else if (which_one == WS_POLYMORPH)
		{
		    register THING *pp;

		    pp = tp->t_pack;
		    detach(mlist, tp);
		    if (see_monst(tp))
			mvaddch(y, x, chat(y, x));
		    oldch = tp->t_oldch;
		    delta.y = y;
		    delta.x = x;
		    new_monster(tp, monster = rnd(26) + 'A', &delta);
		    if (see_monst(tp))
			mvaddch(y, x, monster);
		    tp->t_oldch = oldch;
		    tp->t_pack = pp;
		    ws_know[WS_POLYMORPH] |= (monster != omonst);
		}
		else if (which_one == WS_CANCEL)
		{
		    tp->t_flags |= ISCANC;
		    tp->t_flags &= ~(ISINVIS|CANHUH);
		    tp->t_disguise = tp->t_type;
		}
		else
		{
		    if (see_monst(tp))
			mvaddch(y, x, tp->t_oldch);
		    if (which_one == WS_TELAWAY)
		    {
		    	tp->t_oldch = '@';
			do
			{
			    rm = rnd_room();
			    new_yx = tp->t_pos;
			    rnd_pos(&rooms[rm], &new_yx);
			}  while (!(isfloor(winat(new_yx.y, new_yx.x))));
			tp->t_pos = new_yx;
			if (see_monst(tp))
			    mvaddch(tp->t_pos.y, tp->t_pos.x, tp->t_disguise);
			else if (on(player, SEEMONST))
			{
			    standout();
			    mvaddch(tp->t_pos.y, tp->t_pos.x, tp->t_disguise);
			    standend();
			}
		    }
		    else /* it MUST BE at WS_TELTO */
		    {
			tp->t_pos.y = hero.y + delta.y;
			tp->t_pos.x = hero.x + delta.x;
		    }
		    if (tp->t_type == 'F')
			player.t_flags &= ~ISHELD;
		    if (tp->t_pos.y != y || tp->t_pos.x != x)
			tp->t_oldch = mvinch(tp->t_pos.y, tp->t_pos.x);
		}
		tp->t_dest = &hero;
		tp->t_flags |= ISRUN;
	    }
	}
	when WS_MISSILE:
	{
	    THING bolt;

	    ws_know[WS_MISSILE] = TRUE;
	    bolt.o_type = '*';
	    bolt.o_hurldmg = "1d8";
	    bolt.o_hplus = 1000;
	    bolt.o_dplus = 1;
	    bolt.o_flags = ISMISL;
	    if (cur_weapon != NULL)
		bolt.o_launch = cur_weapon->o_which;
	    do_motion(&bolt, delta.y, delta.x);
	    if ((tp = moat(bolt.o_pos.y, bolt.o_pos.x)) != NULL && !save_throw(VS_MAGIC, tp))
		    hit_monster(unc(bolt.o_pos), &bolt);
	    else 
		msg("the missle vanishes with a puff of smoke");
	}
	when WS_HIT:
	    delta.y += hero.y;
	    delta.x += hero.x;
	    if ((tp = moat(delta.y, delta.x)) != NULL)
	    {
		if (rnd(20) == 0)
		{
		    obj->o_damage = "3d8";
		    obj->o_dplus = 9;
		}
		else
		{
		    obj->o_damage = "2d8";
		    obj->o_dplus = 4;
		}
		fight(&delta, tp->t_type, obj, FALSE);
	    }
	when WS_HASTE_M:
	case WS_SLOW_M:
	    y = hero.y;
	    x = hero.x;
	    while (step_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ((tp = moat(y, x)) != NULL)
	    {
		if (which_one == WS_HASTE_M)
		{
		    if (on(*tp, ISSLOW))
			tp->t_flags &= ~ISSLOW;
		    else
			tp->t_flags |= ISHASTE;
		}
		else
		{
		    if (on(*tp, ISHASTE))
			tp->t_flags &= ~ISHASTE;
		    else
			tp->t_flags |= ISSLOW;
		    tp->t_turn = TRUE;
		}
		delta.y = y;
		delta.x = x;
		start_run(&delta);
	    }
	when WS_ELECT:
	case WS_FIRE:
	case WS_COLD:
	    if (which_one == WS_ELECT)
		name = "bolt";
	    else if (which_one == WS_FIRE)
		name = "flame";
	    else
		name = "ice";
	    fire_bolt(&hero, &delta, name);
	    ws_know[which_one] = TRUE;
#ifdef DEBUG
	otherwise:
	    msg("what a bizarre schtick!");
#endif
    }
    if (--obj->o_charges < 0)
		obj->o_charges = 0;
}

/*
 * drain:
 *	Do drain hit points from player schtick
 */
drain()
{
    THING *mp;
    register int cnt;
    register struct room *corp;
    register THING **dp;
    register bool inpass;
    THING *drainee[40];

    /*
     * First cnt how many things we need to spread the hit points among
     */
    cnt = 0;
    if (chat(hero.y, hero.x) == DOOR)
	corp = &passages[flat(hero.y, hero.x) & F_PNUM];
    else
	corp = NULL;
    inpass = (proom->r_flags & ISGONE);
    dp = drainee;
    for (mp = mlist; mp != NULL; mp = next(mp))
	if (mp->t_room == proom || mp->t_room == corp ||
	    (inpass && chat(mp->t_pos.y, mp->t_pos.x) == DOOR &&
	    &passages[flat(mp->t_pos.y, mp->t_pos.x) & F_PNUM] == proom))
		*dp++ = mp;
    if ((cnt = dp - drainee) == 0)
    {
	msg("you have a tingling feeling");
	return;
    }
    *dp = NULL;
    pstats.s_hpt /= 2;
    cnt = pstats.s_hpt / cnt + 1;
    /*
     * Now zot all of the monsters
     */
    for (dp = drainee; *dp; dp++)
    {
	mp = *dp;
	if ((mp->t_stats.s_hpt -= cnt) <= 0)
	    killed(mp, see_monst(mp));
	else
	    start_run(&mp->t_pos);
    }
}

/*
 * fire_bolt:
 *	Fire a bolt in a given direction from a specific starting place
 */
fire_bolt(start, dir, name)
coord *start, *dir;
char *name;
{
    register byte dirch, ch;
    register THING *tp;
    register bool hit_hero, used, changed;
    register int i, j;
    coord pos;
	struct {
		coord s_pos;
		byte s_under;
	} spotpos[BOLT_LENGTH*2];
    THING bolt;
    bool is_frost;

    is_frost = (strcmp(name, "frost") == 0);
    bolt.o_type = WEAPON;
    bolt.o_which = FLAME;
    bolt.o_damage = bolt.o_hurldmg = "6d6";
    bolt.o_hplus = 30;
    bolt.o_dplus = 0;
    w_names[FLAME] = name;
    switch (dir->y + dir->x) {
	when 0: dirch = '/';
	when 1: case -1: dirch = (dir->y == 0 ? '-' : '|');
	when 2: case -2: dirch = '\\';
    }
	pos = *start;
    hit_hero = (start != &hero);
    used = FALSE;
    changed = FALSE;
    for (i = 0; i < BOLT_LENGTH && !used; i++) {
		pos.y += dir->y;
		pos.x += dir->x;
		ch = winat(pos.y, pos.x);
		spotpos[i].s_pos = pos;
		if ((spotpos[i].s_under = mvinch(pos.y, pos.x)) == dirch)
			spotpos[i].s_under = 0;
		switch (ch) {
	    case DOOR:
	    case HWALL:
	    case VWALL:
	    case ULWALL:
	    case URWALL:
	    case LLWALL:
	    case LRWALL:
	    case ' ':
			if (!changed)
			    hit_hero = !hit_hero;
			changed = FALSE;
			dir->y = -dir->y;
			dir->x = -dir->x;
			i--;
			msg("the %s bounces", name);
			break;
	    default:
			if (!hit_hero && (tp = moat(pos.y, pos.x)) != NULL) {
			    hit_hero = TRUE;
			    changed = !changed;
			    if (tp->t_oldch != '@')
			        tp->t_oldch = chat(pos.y, pos.x);
			    if (!save_throw(VS_MAGIC, tp) || is_frost) {
					bolt.o_pos = pos;
					used = TRUE;
					if (tp->t_type == 'D' && strcmp(name, "flame") == 0)
					    msg("the flame bounces off the dragon");
					else {
					    hit_monster(unc(pos), &bolt);
						if (mvinch(unc(pos)) != dirch)
							spotpos[i].s_under = mvinch(unc(pos));
					}
			    } else if (ch != 'X' || tp->t_disguise == 'X') {
					if (start == &hero)
					    start_run(&pos);
					msg("the %s whizzes past the %s", 
						name, monsters[ch-'A'].m_name);
			    }
			} else if (hit_hero && ce(pos, hero)) {
			    hit_hero = FALSE;
			    changed = !changed;
			    if (!save(VS_MAGIC)) {
			    	if (is_frost) {
					    msg("You are frozen by a blast of frost%s.", noterse(" from the Ice Monster"));
			    	    if (no_command < 20)
							no_command += spread(7);
			    	} else if ((pstats.s_hpt -= roll(6, 6)) <= 0)
					    if (start == &hero)
							death('b');
					    else
							death(moat(start->y, start->x)->t_type);
					used = TRUE;
					if (!is_frost)
					    msg("you are hit by the %s", name);
			    } else
					msg("the %s whizzes by you", name);
			}
			if (is_frost)
				blue();
			else
				red();
			tick_pause();
			mvaddch(pos.y, pos.x, dirch);
			standend();
		}
    }
    for (j = 0; j < i; j++) {
		tick_pause();
		if (spotpos[j].s_under)
			mvaddch(spotpos[j].s_pos.y, spotpos[j].s_pos.x, spotpos[j].s_under);
	}
}

/*
 * charge_str:
 *	Return an appropriate string for a wand charge
 */
char *
charge_str(obj)
register THING *obj;
{
    static char buf[20];

    if (!(obj->o_flags & ISKNOW))
	buf[0] = '\0';
    else 
	sprintf(buf, " [%d charges]", obj->o_charges);
    return buf;
}
