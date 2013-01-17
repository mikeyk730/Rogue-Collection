//endit: Exit the program abnormally.
endit();

//Random number generator - adapted from the FORTRAN version in "Software Manual for the Elementary Functions" by W.J. Cody, Jr and William Waite.
long ran();

//rnd: Pick a very random number.
rnd(int range);

//roll: Roll a number of dice
roll(int number, int sides);

//playit: The main loop of the program.  Loop until the game is over, refreshing things and looking at the proper times.
playit(char *sname);

//quit: Have player make certain, then exit.
quit();

//leave: Leave quickly, but courteously
leave();

//fatal: exit with a message
fatal(char *msg);
fatal_arg(char *msg, int arg);
