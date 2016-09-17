void daemon(void (*func)(), int arg);
void daemon(void (*func)(int), int arg);

//do_daemons: Run all the daemons, passing the argument to the function.
void do_daemons();

//fuse: Start a fuse to go off in a certain number of turns
void fuse(void (*func)(), int arg, int time);
void fuse(void (*func)(int), int arg, int time);

//lengthen: Increase the time until a fuse goes off
void lengthen(void (*func)(), int xtime);
void lengthen(void (*func)(int), int xtime);

//extinguish: Put out a fuse
void extinguish(void (*func)());
void extinguish(void (*func)(int));

//do_fuses: Decrement counters and start needed fuses
void do_fuses();