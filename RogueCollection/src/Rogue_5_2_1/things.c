/*
 * Contains functions for dealing with things like potions, scrolls,
 * and other items.
 *
 * @(#)things.c	4.26 (Berkeley) 5/18/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"

bool got_genocide = FALSE;

/*
 * inv_name:
 *	Return the name of something as it would appear in an
 *	inventory.
 */
char *
inv_name(obj, drop)
register THING *obj;
register bool drop;
{
    register char *pb;

    pb = prbuf;
    switch (obj->o_type)
    {
	case SCROLL:
	    if (obj->o_count == 1)
	    {
		strcpy(pb, "A scroll ");
		pb = &prbuf[9];
	    }
	    else
	    {
		sprintf(pb, "%d scrolls ", obj->o_count);
		pb = &prbuf[strlen(prbuf)];
	    }
	    if (s_know[obj->o_which])
		sprintf(pb, "of %s", s_magic[obj->o_which].mi_name);
	    else if (s_guess[obj->o_which])
		sprintf(pb, "called %s", s_guess[obj->o_which]);
	    else
		sprintf(pb, "titled '%s'", s_names[obj->o_which]);
        when POTION:
	    if (obj->o_count == 1)
	    {
		strcpy(pb, "A potion ");
		pb = &prbuf[9];
	    }
	    else
	    {
		sprintf(pb, "%d potions ", obj->o_count);
		pb = &pb[strlen(prbuf)];
	    }
	    if (p_know[obj->o_which])
		sprintf(pb, "of %s(%s)", p_magic[obj->o_which].mi_name,
		    p_colors[obj->o_which]);
	    else if (p_guess[obj->o_which])
		sprintf(pb, "called %s(%s)", p_guess[obj->o_which],
		    p_colors[obj->o_which]);
	    else if (obj->o_count == 1)
		sprintf(prbuf, "A%s %s potion", vowelstr(p_colors[obj->o_which]),
		    p_colors[obj->o_which]);
	    else
		sprintf(prbuf, "%d %s potions", obj->o_count,
		    p_colors[obj->o_which]);
	when FOOD:
	    if (obj->o_which == 1)
		if (obj->o_count == 1)
		    sprintf(pb, "A%s %s", vowelstr(fruit), fruit);
		else
		    sprintf(pb, "%d %ss", obj->o_count, fruit);
	    else
		if (obj->o_count == 1)
		    strcpy(pb, "Some food");
		else
		    sprintf(pb, "%d rations of food", obj->o_count);
	when WEAPON:
	    if (obj->o_count > 1)
		sprintf(pb, "%d ", obj->o_count);
	    else
		sprintf(pb, "A%s ", vowelstr(w_names[obj->o_which]));
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & ISKNOW)
		sprintf(pb, "%s %s", num(obj->o_hplus, obj->o_dplus, WEAPON),
		    w_names[obj->o_which]);
	    else
		sprintf(pb, "%s", w_names[obj->o_which]);
	    if (obj->o_count > 1)
		strcat(pb, "s");
	when ARMOR:
	    if (obj->o_flags & ISKNOW)
	    {
		sprintf(pb, "%s %s [",
		    num(a_class[obj->o_which] - obj->o_ac, 0, ARMOR),
		    a_names[obj->o_which]);
		if (!terse)
		    strcat(pb, "armor class ");
		pb = &prbuf[strlen(prbuf)];
		sprintf(pb, "%d]", obj->o_ac);
	    }
	    else
		sprintf(pb, "%s", a_names[obj->o_which]);
	when AMULET:
	    strcpy(pb, "The Amulet of Yendor");
	when STICK:
	    sprintf(pb, "A%s %s ", vowelstr(ws_type[obj->o_which]),
		ws_type[obj->o_which]);
	    pb = &prbuf[strlen(prbuf)];
	    if (ws_know[obj->o_which])
		sprintf(pb, "of %s%s(%s)", ws_magic[obj->o_which].mi_name,
		    charge_str(obj), ws_made[obj->o_which]);
	    else if (ws_guess[obj->o_which])
		sprintf(pb, "called %s(%s)", ws_guess[obj->o_which],
		    ws_made[obj->o_which]);
	    else
		sprintf(pb = &prbuf[1], "%s %s %s",
		    vowelstr(ws_made[obj->o_which]), ws_made[obj->o_which],
		    ws_type[obj->o_which]);
        when RING:
	    if (r_know[obj->o_which])
		sprintf(pb, "A%s ring of %s(%s)", ring_num(obj),
		    r_magic[obj->o_which].mi_name, r_stones[obj->o_which]);
	    else if (r_guess[obj->o_which])
		sprintf(pb, "A ring called %s(%s)",
		    r_guess[obj->o_which], r_stones[obj->o_which]);
	    else
		sprintf(pb, "A%s %s ring", vowelstr(r_stones[obj->o_which]),
		    r_stones[obj->o_which]);
	when GOLD:
		sprintf(pb, "%d pieces of gold", obj->o_goldval);
#ifdef WIZARD
	otherwise:
	    debug("Picked up something funny %s", unctrol(obj->o_type));
	    sprintf(pb, "Something bizarre %s", unctrol(obj->o_type));
#endif
    }
    if (obj == cur_armor)
	strcat(pb, " (being worn)");
    if (obj == cur_weapon)
	strcat(pb, " (weapon in hand)");
    if (obj == cur_ring[LEFT])
	strcat(pb, " (on left hand)");
    else if (obj == cur_ring[RIGHT])
	strcat(pb, " (on right hand)");
    if (drop && isupper(prbuf[0]))
	prbuf[0] = tolower(prbuf[0]);
    else if (!drop && islower(*prbuf))
	*prbuf = toupper(*prbuf);
    return prbuf;
}

/*
 * drop:
 *	Put something down
 */
drop()
{
    register char ch;
    register THING *nobj, *op;

    ch = chat(hero.y, hero.x);
    if (ch != FLOOR && ch != PASSAGE)
    {
	after = FALSE;
	msg("there is something there already");
	return;
    }
    if ((op = get_item("drop", 0)) == NULL)
	return;
    if (!dropcheck(op))
	return;
    /*
     * Take it out of the pack
     */
    if (op->o_count >= 2 && op->o_type != WEAPON)
    {
	op->o_count--;
	nobj = new_item();
	*nobj = *op;
	nobj->o_count = 1;
	op = nobj;
	if (op->o_group != 0)
		inpack++;
    }
    else
	detach(pack, op);
    inpack--;
    /*
     * Link it into the level object list
     */
    attach(lvl_obj, op);
    chat(hero.y, hero.x) = op->o_type;
    flat(hero.y, hero.x) |= F_DROPPED;
    op->o_pos = hero;
    if (op->o_type == AMULET)
	amulet = FALSE;
    msg("dropped %s", inv_name(op, TRUE));
}

/*
 * dropcheck:
 *	Do special checks for dropping or unweilding|unwearing|unringing
 */
dropcheck(op)
register THING *op;
{
    if (op == NULL)
	return TRUE;
    if (op != cur_armor && op != cur_weapon
	&& op != cur_ring[LEFT] && op != cur_ring[RIGHT])
	    return TRUE;
    if (op->o_flags & ISCURSED)
    {
	msg("you can't.  It appears to be cursed");
	return FALSE;
    }
    if (op == cur_weapon)
	cur_weapon = NULL;
    else if (op == cur_armor)
    {
	waste_time();
	cur_armor = NULL;
    }
    else
    {
	cur_ring[op == cur_ring[LEFT] ? LEFT : RIGHT] = NULL;
	switch (op->o_which)
	{
	    case R_ADDSTR:
		chg_str(-op->o_ac);
		break;
	    case R_SEEINVIS:
		unsee();
		extinguish(unsee);
		break;
	}
    }
    return TRUE;
}

/*
 * new_thing:
 *	Return a new thing
 */
THING *
new_thing()
{
    register THING *cur;
    register int j, k;

    cur = new_item();
    cur->o_hplus = cur->o_dplus = 0;
    strcpy(cur->o_damage,"0d0");
    strcpy(cur->o_hurldmg,"0d0");
    cur->o_ac = 11;
    cur->o_count = 1;
    cur->o_group = 0;
    cur->o_flags = 0;
    /*
     * Decide what kind of object it will be
     * If we haven't had food for a while, let it be food.
     */
    switch (no_food > 3 ? 2 : pick_one(things, NUMTHINGS))
    {
	case 0:
	    cur->o_type = POTION;
	    cur->o_which = pick_one(p_magic, MAXPOTIONS);
	when 1:
	    cur->o_type = SCROLL;
	    cur->o_which = pick_one(s_magic, MAXSCROLLS);
	    /*
	     * Only one genocide scroll allowed per game, so if it's
	     * the second one, then turn it into a identify scroll
	     */
	    if (cur->o_which == S_GENOCIDE)
		if (got_genocide)
		    cur->o_which = S_IDENT;
		else
		    got_genocide = TRUE;
	when 2:
	    no_food = 0;
	    cur->o_type = FOOD;
	    if (rnd(10) != 0)
		cur->o_which = 0;
	    else
		cur->o_which = 1;
	when 3:
	    cur->o_type = WEAPON;
	    cur->o_which = rnd(MAXWEAPONS);
	    init_weapon(cur, cur->o_which);
	    if ((k = rnd(100)) < 10)
	    {
		cur->o_flags |= ISCURSED;
		cur->o_hplus -= rnd(3) + 1;
	    }
	    else if (k < 15)
		cur->o_hplus += rnd(3) + 1;
	when 4:
	    cur->o_type = ARMOR;
	    for (j = 0, k = rnd(100); j < MAXARMORS; j++)
		if (k < a_chances[j])
		    break;
#ifdef WIZARD
	    if (j == MAXARMORS)
	    {
		debug("Picked a bad armor %d", k);
		j = 0;
	    }
#endif
	    cur->o_which = j;
	    cur->o_ac = a_class[j];
	    if ((k = rnd(100)) < 20)
	    {
		cur->o_flags |= ISCURSED;
		cur->o_ac += rnd(3) + 1;
	    }
	    else if (k < 28)
		cur->o_ac -= rnd(3) + 1;
	when 5:
	    cur->o_type = RING;
	    cur->o_which = pick_one(r_magic, MAXRINGS);
	    switch (cur->o_which)
	    {
		case R_ADDSTR:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		    if ((cur->o_ac = rnd(3)) == 0)
		    {
			cur->o_ac = -1;
			cur->o_flags |= ISCURSED;
		    }
		when R_AGGR:
		case R_TELEPORT:
		    cur->o_flags |= ISCURSED;
	    }
	when 6:
	    cur->o_type = STICK;
	    cur->o_which = pick_one(ws_magic, MAXSTICKS);
	    fix_stick(cur);
#ifdef WIZARD
	otherwise:
	    debug("Picked a bad kind of object");
	    wait_for(' ');
#endif
    }
    return cur;
}

/*
 * pick_one:
 *	Pick an item out of a list of nitems possible magic items
 */
pick_one(magic, nitems)
register struct magic_item *magic;
int nitems;
{
    register struct magic_item *end;
    register int i;
    register struct magic_item *start;

    start = magic;
    for (end = &magic[nitems], i = rnd(100); magic < end; magic++)
	if (i < magic->mi_prob)
	    break;
    if (magic == end)
    {
#ifdef WIZARD
	if (wizard)
	{
	    msg("bad pick_one: %d from %d items", i, nitems);
	    for (magic = start; magic < end; magic++)
		msg("%s: %d%%", magic->mi_name, magic->mi_prob);
	}
#endif
	magic = start;
    }
    return magic - start;
}

/*
 * discovered:
 *	list what the player has discovered in this game of a certain type
 */
static int line_cnt = 0;

static bool newpage = FALSE;

static char *lastfmt, *lastarg;

discovered()
{
    register char ch;
    register bool disc_list;

    do {
	disc_list = FALSE;
	if (!terse)
	    addmsg("for ");
	addmsg("what type");
	if (!terse)
	    addmsg(" of object do you want a list");
	msg("? (* for all)");
	ch = readchar();
	switch (ch)
	{
	    case ESCAPE:
		msg("");
		return;
	    case POTION:
	    case SCROLL:
	    case RING:
	    case STICK:
	    case '*':
		disc_list = TRUE;
		break;
	    default:
		if (terse)
		    msg("Not a type");
		else
		    msg("Please type one of %c%c%c%c (ESCAPE to quit)", POTION, SCROLL, RING, STICK);
	}
    } while (!disc_list);
    if (ch == '*')
    {
	print_disc(POTION);
	add_line("");
	print_disc(SCROLL);
	add_line("");
	print_disc(RING);
	add_line("");
	print_disc(STICK);
	end_line();
    }
    else
    {
	print_disc(ch);
	end_line();
    }
}

/*
 * print_disc:
 *	Print what we've discovered of type 'type'
 */

#define MAX(a,b,c,d)	(a > b ? (a > c ? (a > d ? a : d) : (c > d ? c : d)) : (b > c ? (b > d ? b : d) : (c > d ? c : d)))

print_disc(type)
char type;
{
    register bool *know = NULL;
    register char **guess = NULL;
    register int i, maxnum = 0, num_found;
    static THING obj;
    static short order[MAX(MAXSCROLLS, MAXPOTIONS, MAXRINGS, MAXSTICKS)];

    switch (type)
    {
	case SCROLL:
	    maxnum = MAXSCROLLS;
	    know = s_know;
	    guess = s_guess;
	    break;
	case POTION:
	    maxnum = MAXPOTIONS;
	    know = p_know;
	    guess = p_guess;
	    break;
	case RING:
	    maxnum = MAXRINGS;
	    know = r_know;
	    guess = r_guess;
	    break;
	case STICK:
	    maxnum = MAXSTICKS;
	    know = ws_know;
	    guess = ws_guess;
	    break;
    }
    set_order(order, maxnum);
    obj.o_count = 1;
    obj.o_flags = 0;
    num_found = 0;
    for (i = 0; i < maxnum; i++)
	if (know[order[i]] || guess[order[i]])
	{
	    obj.o_type = type;
	    obj.o_which = order[i];
	    add_line("%s", inv_name(&obj, FALSE));
	    num_found++;
	}
    if (num_found == 0)
	add_line(nothing(type));
}

/*
 * set_order:
 *	Set up order for list
 */
set_order(order, numthings)
short *order;
int numthings;
{
    register int i, r, t;

    for (i = 0; i< numthings; i++)
	order[i] = i;

    for (i = numthings; i > 0; i--)
    {
	r = rnd(i);
	t = order[i - 1];
	order[i - 1] = order[r];
	order[r] = t;
    }
}

/*
 * add_line:
 *	Add a line to the list of discoveries
 */
/* VARARGS1 */
add_line(fmt, arg)
char *fmt, *arg;
{
    if (line_cnt == 0)
    {
	    wclear(hw);
	    if (slow_invent)
		mpos = 0;
    }
    if (slow_invent)
    {
	if (*fmt != '\0')
	    msg(fmt, arg);
	line_cnt++;
    }
    else
    {
	if (line_cnt >= LINES - 1 || fmt == NULL)
	{
	    mvwaddstr(hw, LINES - 1, 0, "--Press space to continue--");
	    wrefresh(hw);
	    w_wait_for(hw,' ');
	    clearok(curscr, TRUE);
	    wclear(hw);
            touchwin(stdscr);
	    newpage = TRUE;
	    line_cnt = 0;
	}
	if (fmt != NULL && !(line_cnt == 0 && *fmt == '\0'))
	{
	    mvwprintw(hw, line_cnt++, 0, fmt, arg);
	    lastfmt = fmt;
	    lastarg = arg;
	}
    }
}

/*
 * end_line:
 *	End the list of lines
 */
end_line()
{
    if (!slow_invent)
	if (line_cnt == 1 && !newpage)
	{
	    mpos = 0;
	    msg(lastfmt, lastarg);
	}
	else
	    add_line(NULL);
    line_cnt = 0;
    newpage = FALSE;
}

/*
 * nothing:
 *	Set up prbuf so that message for "nothing found" is there
 */
char *
nothing(type)
register char type;
{
    register char *sp, *tystr = NULL;

    if (terse)
	sprintf(prbuf, "Nothing");
    else
	sprintf(prbuf, "Haven't discovered anything");
    if (type != '*')
    {
	sp = &prbuf[strlen(prbuf)];
	switch (type)
	{
	    case POTION: tystr = "potion";
	    when SCROLL: tystr = "scroll";
	    when RING: tystr = "ring";
	    when STICK: tystr = "stick";
	}
	sprintf(sp, " about any %ss", tystr);
    }
    return prbuf;
}
