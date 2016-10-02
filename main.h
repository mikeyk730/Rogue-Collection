//endit: Exit the program abnormally.
void endit();

int get_seed();

//rnd: Pick a very random number.
int rnd(int range);

//roll: Roll a number of dice
int roll(int number, int sides);

//playit: The main loop of the program.  Loop until the game is over, refreshing things and looking at the proper times.
void playit();

//do_quit: Have player make certain, then exit.
bool do_quit();

//leave: Leave quickly, but courteously
void leave();

//fatal: exit with a message
void fatal(char *msg, ...);
