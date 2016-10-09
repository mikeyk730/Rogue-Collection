#include "curses.h"
#include "rogue.h"

/*
 * new_level:
 *	Dig and draw a new level
 *
 * @(#)new_level.c	3.7 (Berkeley) 6/2/81
 */

new_level()
{
    register int rm, i;
    register char ch;
    coord stairs;

    if (level > max_level)
	max_level = level;
    wclear(cw);
    wclear(mw);
    clear();
    status();
    /*
     * Free up the monsters on the last level
     */
    free_list(mlist);
    do_rooms();				/* Draw rooms */
    do_passages();			/* Draw passages */
    no_food++;
    put_things();			/* Place objects (if any) */
    /*
     * Place the staircase down.
     */
    do {
        rm = rnd_room();
	rnd_pos(&rooms[rm], &stairs);
    } until (winat(stairs.y, stairs.x) == FLOOR);
    addch(STAIRS);
    /*
     * Place the traps
     */
    if (rnd(10) < level)
    {
	ntraps = rnd(level/4)+1;
	if (ntraps > MAXTRAPS)
	    ntraps = MAXTRAPS;
	i = ntraps;
	while (i--)
	{
	    do
	    {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &stairs);
	    } until (winat(stairs.y, stairs.x) == FLOOR);
	    switch(rnd(6))
	    {
		case 0: ch = TRAPDOOR;
		when 1: ch = BEARTRAP;
		when 2: ch = SLEEPTRAP;
		when 3: ch = ARROWTRAP;
		when 4: ch = TELTRAP;
		when 5: ch = DARTTRAP;
	    }
	    addch(TRAP);
	    traps[i].tr_type = ch;
	    traps[i].tr_flags = 0;
	    traps[i].tr_pos = stairs;
	}
    }
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    }
    until(winat(hero.y, hero.x) == FLOOR);
    light(&hero);
    wmove(cw, hero.y, hero.x);
    waddch(cw, PLAYER);
}

/*
 * Pick a room that is really there
 */

rnd_room()
{
    register int rm;

    do
    {
	rm = rnd(MAXROOMS);
    } while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * put_things:
 *	put potions and scrolls on this level
 */

put_things()
{
    register int i;
    register struct linked_list *item;
    register struct object *cur;
    register int rm;
    coord tp;

    /*
     * Throw away stuff left on the previous level (if anything)
     */
    free_list(lvl_obj);
    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
	return;
    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (i = 0; i < MAXOBJ; i++)
	if (rnd(100) < 35)
	{
	    /*
	     * Pick a new object and link it in the list
	     */
	    item = new_thing();
	    attach(lvl_obj, item);
	    cur = (struct object *) ldata(item);
	    /*
	     * Put it somewhere
	     */
	    rm = rnd_room();
	    do {
		rnd_pos(&rooms[rm], &tp);
	    } until (winat(tp.y, tp.x) == FLOOR);
	    mvaddch(tp.y, tp.x, cur->o_type);
	    cur->o_pos = tp;
	}
    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground
     */
    if (level > 25 && !amulet)
    {
	item = new_item(sizeof *cur);
	attach(lvl_obj, item);
	cur = (struct object *) ldata(item);
	cur->o_hplus = cur->o_dplus = 0;
	strcpy(cur->o_damage,"0d0");
        strcpy(cur->o_hurldmg, "0d0");
	cur->o_ac = 11;
	cur->o_type = AMULET;
	/*
	 * Put it somewhere
	 */
	rm = rnd_room();
	do {
	    rnd_pos(&rooms[rm], &tp);
	} until (winat(tp.y, tp.x) == FLOOR);
	mvaddch(tp.y, tp.x, cur->o_type);
	cur->o_pos = tp;
    }
}
