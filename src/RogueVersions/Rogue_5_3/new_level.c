#include <curses.h>
#include "rogue.h"

#define TREAS_ROOM 20	/* one chance in TREAS_ROOM for a treasure room */
#define MAXTREAS 10	/* maximum number of treasures in a treasure room */
#define MINTREAS 2	/* minimum number of treasures in a treasure room */

/*
 * new_level:
 *	Dig and draw a new level
 *
 * @(#)new_level.c	4.24 (NMT from Berkeley 5.2) 8/25/83
 */
new_level()
{
    register int i;
    register THING *tp;
    register char *sp;
    register THING **mp;

    player.t_flags &= ~ISHELD;	/* unhold when you go down just in case */
    if (level > max_level)
	max_level = level;
    /*
     * Clean things off from last level
     */
    clear();
    for (sp = _level; sp < &_level[MAXCOLS*MAXLINES]; )
	*sp++ = ' ';
    for (sp = _flags; sp < &_flags[MAXCOLS*MAXLINES]; )
	*sp++ = F_REAL;
    for (mp = _monst; mp < &_monst[MAXCOLS*MAXLINES]; )
	*mp++ = NULL;
    clear();
    /*
     * Free up the monsters on the last level
     */
    for (tp = mlist; tp != NULL; tp = next(tp))
	free_list(tp->t_pack);
    free_list(mlist);
    /*
     * Throw away stuff left on the previous level (if anything)
     */
    free_list(lvl_obj);
    do_rooms();				/* Draw rooms */
    do_passages();			/* Draw passages */
    no_food++;
    put_things();			/* Place objects (if any) */
    /*
     * Place the traps
     */
    if (rnd(10) < level)
    {
	ntraps = rnd(level / 4) + 1;
	if (ntraps > MAXTRAPS)
	    ntraps = MAXTRAPS;
	i = ntraps;
	while (i--)
	{
	    /*
	     * not only wouldn't it be NICE to have traps in mazes
	     * (not that we care about being nice), since the trap
	     * number is stored where the passage number is, we
	     * can't actually do it.
	     */
	    do
	    {
		find_floor((struct room *) NULL, &stairs, FALSE, FALSE);
	    } while (chat(stairs.y, stairs.x) != FLOOR);
	    sp = &flat(stairs.y, stairs.x);
	    *sp &= ~F_REAL;
	    *sp |= rnd(NTRAPS);
	}
    }
    /*
     * Place the staircase down.
     */
    i = 0;
    find_floor((struct room *) NULL, &stairs, FALSE, FALSE);
    chat(stairs.y, stairs.x) = STAIRS;
    find_floor((struct room *) NULL, &hero, FALSE, TRUE);
    enter_room(&hero);
    move(hero.y, hero.x);
    addrawch(PLAYER);
    if (on(player, SEEMONST))
	turn_see(FALSE);
    if (on(player, ISTrip))
	visuals();
}

/*
 * rnd_room:
 *	Pick a room that is really there
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
 *	Put potions and scrolls on this level
 */
put_things()
{
    register int i;
    register THING *cur;

    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
	return;
    /*
     * check for treasure rooms, and if so, put it in.
     */
    if (rnd(TREAS_ROOM) == 0)
	treas_room();
    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (i = 0; i < MAXOBJ; i++)
	if (rnd(100) < 35)
	{
	    /*
	     * Pick a new object and link it in the list
	     */
	    cur = new_thing();
	    attach(lvl_obj, cur);
	    /*
	     * Put it somewhere
	     */
	    find_floor((struct room *) NULL, &cur->o_pos, FALSE, FALSE);
	    chat(cur->o_pos.y, cur->o_pos.x) = cur->o_type;
	}
    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground
     */
    if (level >= AMULETLEVEL && !amulet)
    {
	cur = new_item();
	attach(lvl_obj, cur);
	cur->o_hplus = cur->o_dplus = 0;
	cur->o_damage = cur->o_hurldmg = "0d0";
	cur->o_ac = 11;
	cur->o_type = AMULET;
	/*
	 * Put it somewhere
	 */
	find_floor((struct room *) NULL, &cur->o_pos, FALSE, FALSE);
	chat(cur->o_pos.y, cur->o_pos.x) = AMULET;
    }
}

/*
 * treas_room:
 *	Add a treasure room
 */
#define MAXTRIES 10	/* max number of tries to put down a monster */

treas_room()
{
    register int nm;
    register THING *tp;
    register struct room *rp;
    register int spots, num_monst;
    coord mp;

    rp = &rooms[rnd_room()];
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2) - MINTREAS;
    if (spots > (MAXTREAS - MINTREAS))
	spots = (MAXTREAS - MINTREAS);
    num_monst = nm = rnd(spots) + MINTREAS;
    while (nm--)
    {
	find_floor(rp, &mp, 2 * MAXTRIES, FALSE);
	tp = new_thing();
	tp->o_pos = mp;
	attach(lvl_obj, tp);
	chat(mp.y, mp.x) = tp->o_type;
    }

    /*
     * fill up room with monsters from the next level down
     */

    if ((nm = rnd(spots) + MINTREAS) < num_monst + 2)
	nm = num_monst + 2;
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2);
    if (nm > spots)
	nm = spots;
    level++;
    while (nm--)
    {
	spots = 0;
	if (find_floor(rp, &mp, MAXTRIES, TRUE))
	{
	    tp = new_item();
	    new_monster(tp, randmonster(FALSE), &mp);
	    tp->t_flags |= ISMEAN;	/* no sloughers in THIS room */
	    give_pack(tp);
	}
    }
    level--;
}
