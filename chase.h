//runners: Make all the running monsters move.
void runners();

//do_chase: Make one thing chase another.
void do_chase(THING *th);

//see_monst: Return TRUE if the hero can see the monster

int see_monst(THING *mp);

//start_run: Set a monster running after something or stop it from running (for when it dies)
void start_run(coord *runner);

//chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns TRUE if we want to keep on chasing later. FALSE if we reach the goal.
void chase(THING *tp, coord *ee);

//roomin: Find what room some coordinates are in. NULL means they aren't in any room.
struct room *roomin(coord *cp);

//diag_ok: Check to see if the move is legal if it is diagonal
int diag_ok(coord *sp, coord *ep);

//cansee: Returns true if the hero can see a certain coordinate.
int cansee(int y, int x);

//find_dest: find the proper destination for the monster
coord *find_dest(THING *tp);