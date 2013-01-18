//Contains functions for dealing with things that happen in the future.
//@(#)daemon.c5.2 (Berkeley) 6/18/82

#include "rogue.h"
#include "daemon.h"
#include "io.h"

#define EMPTY  0
#define FULL  1
#define DAEMON  -1
#define MAXDAEMONS  20

struct delayed_action
{
  void (*d_func)();
  int d_arg;
  int d_time;
} d_list[MAXDAEMONS];

//d_slot: Find an empty slot in the daemon/fuse list
struct delayed_action *d_slot()
{
  struct delayed_action *dev;

  for (dev = d_list; dev<&d_list[MAXDAEMONS]; dev++) if (dev->d_func==EMPTY) return dev;
#ifdef DEBUG
  debug("Ran out of fuse slots");
#endif
  return NULL;
}

//find_slot: Find a particular slot in the table
struct delayed_action *find_slot(void (*func)())
{
  struct delayed_action *dev;

  for (dev = d_list; dev<&d_list[MAXDAEMONS]; dev++) if (func==dev->d_func) return dev;
  return NULL;
}

//daemon: Start a daemon, takes a function.
void daemon(void (*func)(), int arg)
{
  struct delayed_action *dev;

  dev = d_slot();
  dev->d_func = func;
  dev->d_arg = arg;
  dev->d_time = DAEMON;
}

//do_daemons: Run all the daemons, passing the argument to the function.
void do_daemons()
{
  struct delayed_action *dev;

  //Loop through the devil list, Executing each one, giving it the proper arguments
  for (dev = d_list; dev<&d_list[MAXDAEMONS]; dev++)
  if (dev->d_time==DAEMON && dev->d_func!=EMPTY) (*dev->d_func)(dev->d_arg);
}

//fuse: Start a fuse to go off in a certain number of turns
void fuse(void (*func)(), int arg, int time)
{
  struct delayed_action *wire;

  wire = d_slot();
  wire->d_func = func;
  wire->d_arg = arg;
  wire->d_time = time;
}

//lengthen: Increase the time until a fuse goes off
void lengthen(void (*func)(), int xtime)
{
  struct delayed_action *wire;

  if ((wire = find_slot(func))==NULL) return;
  wire->d_time += xtime;
}

//extinguish: Put out a fuse
void extinguish(void (*func)())
{
  struct delayed_action *wire;

  if ((wire = find_slot(func))==NULL) return;
  wire->d_func = EMPTY;
}

//do_fuses: Decrement counters and start needed fuses
void do_fuses()
{
  struct delayed_action *wire;

  //Step through the list
  //Decrementing counters and starting things we want.  We also need to remove the fuse from the list once it has gone off.
  for (wire = d_list; wire<&d_list[MAXDAEMONS]; wire++)
  {
    if (wire->d_func!=EMPTY && wire->d_time>0 && --wire->d_time==0)
    {
      (*wire->d_func)(wire->d_arg);
      wire->d_func = EMPTY;
    }
  }
}
