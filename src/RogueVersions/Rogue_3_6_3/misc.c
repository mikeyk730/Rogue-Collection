/*
 * all sorts of miscellaneous routines
 *
 * @(#)misc.c	3.13 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "curses.h"
#include "rogue.h"
#include <ctype.h>

/*
 * tr_name:
 *	print the name of a trap
 */

char *
tr_name(int ch)
{
    char *s = "";

    switch (ch)
    {
	case TRAPDOOR:
	    s = terse ? "A trapdoor." : "You found a trapdoor.";
	when BEARTRAP:
	    s = terse ? "A beartrap." : "You found a beartrap.";
	when SLEEPTRAP:
	    s = terse ? "A sleeping gas trap.":"You found a sleeping gas trap.";
	when ARROWTRAP:
	    s = terse ? "An arrow trap." : "You found an arrow trap.";
	when TELTRAP:
	    s = terse ? "A teleport trap." : "You found a teleport trap.";
	when DARTTRAP:
	    s = terse ? "A dart trap." : "You found a poison dart trap.";
    }
    return s;
}

int offmap(int y, int x)
{
	if ((y > 0 && y < LINES - 1) && (x >= 0 && x < COLS))
	{
		return 0;
	}

	return 1;
}

/*
 * Look:
 *	A quick glance all around the player
 */

void
look(int wakeup)
{
    int x, y;
    int ch;
    int oldx, oldy;
    int inpass;
    int passcount = 0;
    struct room *rp;
    int ey, ex;

    getyx(cw, oldy, oldx);
    if (oldrp != NULL && (oldrp->r_flags & ISDARK) && off(player, ISBLIND))
    {
	for (x = oldpos.x - 1; x <= oldpos.x + 1; x++)
	    for (y = oldpos.y - 1; y <= oldpos.y + 1; y++)
	    {
		if (offmap(y, x)) continue; /* mdk: bounds check */
		if ((y != hero.y || x != hero.x) && show(y, x) == FLOOR)
		    mvwaddrawch(cw, y, x, ' ');
		}
    }
    inpass = ((rp = roomin(&hero)) == NULL);
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x++)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= ey; y++)
	{
		if (offmap(y, x)) continue; /* mdk: bounds check */
	    if (y <= 0 || y >= LINES - 1)
		continue;
	    if (ismons(CMVWINCH(mw, y, x)))
	    {
		struct linked_list *it;
		struct thing *tp;

		if (wakeup)
		    it = wake_monster(y, x);
		else
		    it = find_mons(y, x);
		tp = (struct thing *) ldata(it);
		if ((tp->t_oldch = CMVINCH(y, x)) == TRAP)
		    tp->t_oldch =
			(trap_at(y,x)->tr_flags&ISFOUND) ? TRAP : FLOOR;
		if (tp->t_oldch == FLOOR && (rp != NULL) && (rp->r_flags & ISDARK)
		    && off(player, ISBLIND))
			tp->t_oldch = ' ';
	    }
	    /*
	     * Secret doors show as walls
	     */
	    if ((ch = show(y, x)) == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if he is in a passage
	     */
	    if (off(player, ISBLIND))
	    {
		if (y == hero.y && x == hero.x
		 || (inpass && (ch == HWALL || ch == VWALL
		             || ch == LLWALL || ch == LRWALL || ch == URWALL || ch == ULWALL
             )))
			continue;
	    }
	    else if (y != hero.y || x != hero.x)
		continue;
	    wmove(cw, y, x);
        PC_GFX_PASSGE_COLOR(y, x, ch, 0x70);
        waddrawch(cw, ch);
        PC_GFX_NOCOLOR(cw, 0x70);
	    if (door_stop && !firstmove && running)
	    {
		switch (runch)
		{
		    case 'h':
			if (x == ex)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == ey)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		switch (ch)
		{
		    case DOOR:
			if (x == hero.x || y == hero.y)
			    running = FALSE;
			break;
		    case PASSAGE:
			if (x == hero.x || y == hero.y)
			    passcount++;
			break;
		    case FLOOR:
		    case VWALL:
		    case HWALL:
			PC_GFX_WALL_CASES
		    case ' ':
			break;
		    default:
			running = FALSE;
			break;
		}
	    }
	}
    if (door_stop && !firstmove && passcount > 1)
	running = FALSE;
    PC_GFX_PASSGE_COLOR(hero.y, hero.x, PLAYER, 0x70);
    mvwaddrawch(cw, hero.y, hero.x, PLAYER);
    PC_GFX_NOCOLOR(cw, 0x70);
    wmove(cw, oldy, oldx);
    oldpos = hero;
    oldrp = rp;
}

/*
 * secret_door:
 *	Figure out what a secret door looks like.
 */

int
secretdoor(int y, int x)
{
    int i;
    struct room *rp;
    coord *cpp;
    static coord cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp++, i++)
	if (inroom(rp, cpp))
	    if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
		return(HWALL);
	    else
		return(VWALL);

    return('p');
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */

struct linked_list *
find_obj(int y, int x)
{
    struct linked_list *obj;
    struct object *op;

    for (obj = lvl_obj; obj != NULL; obj = next(obj))
    {
	op = (struct object *) ldata(obj);
	if (op->o_pos.y == y && op->o_pos.x == x)
		return obj;
    }
    sprintf(prbuf, "Non-object %d,%d", y, x);
    debug(prbuf);
    return NULL;
}

/*
 * eat:
 *	She wants to eat something, so let her try
 */

void
eat()
{
    struct linked_list *item;
    struct object *obj;

    if ((item = get_item("eat", FOOD)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != FOOD)
    {
	if (!terse)
	    msg("Ugh, you would get ill if you ate that.");
	else
	    msg("That's Inedible!");
	return;
    }
    inpack--;
    if (obj->o_which == 1)
	msg("My, that was a yummy %s", fruit);
    else
	if (rnd(100) > 70)
	{
	    msg("Yuk, this food tastes awful");
	    pstats.s_exp++;
	    check_level();
	}
	else
	    msg("Yum, that tasted good");
    if ((food_left += HUNGERTIME + rnd(400) - 200) > STOMACHSIZE)
	food_left = STOMACHSIZE;
    hungry_state = 0;
    if (obj == cur_weapon)
	cur_weapon = NULL;
    if (--obj->o_count < 1)
    {
	detach(pack, item);
	discard(item);
    }
    play_sound("eat");
}

/*
 * Used to modify the playes strength
 * it keeps track of the highest it has been, just in case
 */

void
chg_str(int amt)
{
    if (amt == 0)
	return;
    if (amt > 0)
    {
	while (amt--)
	{
	    if (pstats.s_str.st_str < 18)
		pstats.s_str.st_str++;
	    else if (pstats.s_str.st_add == 0)
		pstats.s_str.st_add = rnd(50) + 1;
	    else if (pstats.s_str.st_add <= 50)
		pstats.s_str.st_add = 51 + rnd(24);
	    else if (pstats.s_str.st_add <= 75)
		pstats.s_str.st_add = 76 + rnd(14);
	    else if (pstats.s_str.st_add <= 90)
		pstats.s_str.st_add = 91;
	    else if (pstats.s_str.st_add < 100)
		pstats.s_str.st_add++;
	}
	if (pstats.s_str.st_str > max_stats.s_str.st_str ||
	    (pstats.s_str.st_str == 18 &&
	     pstats.s_str.st_add > max_stats.s_str.st_add))
		max_stats.s_str = pstats.s_str;
    }
    else
    {
	while (amt++)
	{
	    if (pstats.s_str.st_str < 18 || pstats.s_str.st_add == 0)
		pstats.s_str.st_str--;
	    else if (pstats.s_str.st_add < 51)
		pstats.s_str.st_add = 0;
	    else if (pstats.s_str.st_add < 76)
		pstats.s_str.st_add = 1 + rnd(50);
	    else if (pstats.s_str.st_add < 91)
		pstats.s_str.st_add = 51 + rnd(25);
	    else if (pstats.s_str.st_add < 100)
		pstats.s_str.st_add = 76 + rnd(14);
	    else
		pstats.s_str.st_add = 91 + rnd(8);
	}
	if (pstats.s_str.st_str < 3)
	    pstats.s_str.st_str = 3;
    }
}

/*
 * add_haste:
 *	add a haste to the player
 */

void
add_haste(int potion)
{
    if (on(player, ISHASTE))
    {
	msg("You faint from exhaustion.");
	no_command += rnd(8);
	extinguish(nohaste);
    }
    else
    {
	player.t_flags |= ISHASTE;
	if (potion)
	    fuse(nohaste, 0, rnd(4)+4, AFTER);
    }
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */

void
aggravate()
{
    struct linked_list *mi;

    for (mi = mlist; mi != NULL; mi = next(mi))
	runto(&((struct thing *) ldata(mi))->t_pos, &hero);
}

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char *
vowelstr(char *str)
{
    switch (*str)
    {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	    return "n";
	default:
	    return "";
    }
}

/* 
 * see if the object is one of the currently used items
 */
int
is_current(struct object *obj)
{
    if (obj == NULL)
	return FALSE;
    if (obj == cur_armor || obj == cur_weapon || obj == cur_ring[LEFT]
	|| obj == cur_ring[RIGHT])
    {
	msg(terse ? "In use." : "That's already in use.");
	return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
int
get_dir()
{
    char *prompt;
    int gotit;

    if (!terse)
	msg(prompt = "Which direction? ");
    else
	msg(prompt = "Direction: ");
    do
    {
	gotit = TRUE;
	switch (readchar(cw))
	{
	    case 'h': case'H': delta.y =  0; delta.x = -1;
	    when 'j': case'J': delta.y =  1; delta.x =  0;
	    when 'k': case'K': delta.y = -1; delta.x =  0;
	    when 'l': case'L': delta.y =  0; delta.x =  1;
	    when 'y': case'Y': delta.y = -1; delta.x = -1;
	    when 'u': case'U': delta.y = -1; delta.x =  1;
	    when 'b': case'B': delta.y =  1; delta.x = -1;
	    when 'n': case'N': delta.y =  1; delta.x =  1;
	    when ESCAPE: CLEAR_MSG; return FALSE;
	    otherwise:
		mpos = 0;
		msg(prompt);
		gotit = FALSE;
	}
    } until (gotit);
    if (on(player, ISHUH) && rnd(100) > 80)
	do
	{
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    mpos = 0;
    return TRUE;
}
