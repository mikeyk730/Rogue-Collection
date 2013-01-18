//endit: Exit the program abnormally.
void endit();

//Random number generator - adapted from the FORTRAN version in "Software Manual for the Elementary Functions" by W.J. Cody, Jr and William Waite.
long ran();

int srand2();

//rnd: Pick a very random number.
int rnd(int range);

//roll: Roll a number of dice
int roll(int number, int sides);

//playit: The main loop of the program.  Loop until the game is over, refreshing things and looking at the proper times.
void playit(char *sname);

//quit: Have player make certain, then exit.
void quit();

//leave: Leave quickly, but courteously
void leave();

//fatal: exit with a message
void fatal(char *msg, ...);
