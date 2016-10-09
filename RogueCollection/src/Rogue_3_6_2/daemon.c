/*
 * Contains functions for dealing with things that happen in the
 * future.
 *
 * @(#)daemon.c	3.3 (Berkeley) 6/15/81
 */

#include "curses.h"
#include "rogue.h"

#define EMPTY 0
#define DAEMON -1
#define MAXDAEMONS 20

#define _X_ { EMPTY }

struct delayed_action {
    int d_type;
    int (*d_func)();
    int d_arg;
    int d_time;
} d_list[MAXDAEMONS] = {
    _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
    _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, 
};

/*
 * d_slot:
 *	Find an empty slot in the daemon/fuse list
 */
struct delayed_action *
d_slot()
{
    register int i;
    register struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
	if (dev->d_type == EMPTY)
	    return dev;
    debug("Ran out of fuse slots");
    return NULL;
}

/*
 * find_slot:
 *	Find a particular slot in the table
 */

struct delayed_action *
find_slot(func)
register int (*func)();
{
    register int i;
    register struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
	if (dev->d_type != EMPTY && func == dev->d_func)
	    return dev;
    return NULL;
}

/*
 * daemon:
 *	Start a daemon, takes a function.
 */

daemon(func, arg, type)
int (*func)(), arg, type;
{
    register struct delayed_action *dev;

    dev = d_slot();
    dev->d_type = type;
    dev->d_func = func;
    dev->d_arg = arg;
    dev->d_time = DAEMON;
}

/*
 * kill_daemon:
 *	Remove a daemon from the list
 */

kill_daemon(func)
int (*func)();
{
    register struct delayed_action *dev;

    if ((dev = find_slot(func)) == NULL)
	return;
    /*
     * Take it out of the list
     */
    dev->d_type = EMPTY;
}

/*
 * do_daemons:
 *	Run all the daemons that are active with the current flag,
 *	passing the argument to the function.
 */

do_daemons(flag)
register int flag;
{
    register struct delayed_action *dev;

    /*
     * Loop through the devil list
     */
    for (dev = d_list; dev <= &d_list[MAXDAEMONS-1]; dev++)
	/*
	 * Executing each one, giving it the proper arguments
	 */
	if (dev->d_type == flag && dev->d_time == DAEMON)
	    (*dev->d_func)(dev->d_arg);
}

/*
 * fuse:
 *	Start a fuse to go off in a certain number of turns
 */

fuse(func, arg, time, type)
int (*func)(), arg, time, type;
{
    register struct delayed_action *wire;

    wire = d_slot();
    wire->d_type = type;
    wire->d_func = func;
    wire->d_arg = arg;
    wire->d_time = time;
}

/*
 * lengthen:
 *	Increase the time until a fuse goes off
 */

lengthen(func, xtime)
int (*func)();
int xtime;
{
    register struct delayed_action *wire;

    if ((wire = find_slot(func)) == NULL)
	return;
    wire->d_time += xtime;
}

/*
 * extinguish:
 *	Put out a fuse
 */

extinguish(func)
int (*func)();
{
    register struct delayed_action *wire;

    if ((wire = find_slot(func)) == NULL)
	return;
    wire->d_type = EMPTY;
}

/*
 * do_fuses:
 *	Decrement counters and start needed fuses
 */

do_fuses(flag)
register int flag;
{
    register struct delayed_action *wire;

    /*
     * Step though the list
     */
    for (wire = d_list; wire <= &d_list[MAXDAEMONS-1]; wire++)
    {
	/*
	 * Decrementing counters and starting things we want.  We also need
	 * to remove the fuse from the list once it has gone off.
	 */
	if (flag == wire->d_type && wire->d_time > 0 && --wire->d_time == 0)
	{
	    wire->d_type = EMPTY;
	    (*wire->d_func)(wire->d_arg);
	}
     }
}
