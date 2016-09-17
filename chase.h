//runners: Make all the running monsters move.
void runners();

//do_chase: Make one thing chase another.  Returns false if monster has been invalidated
bool do_chase(AGENT *monster);

//see_monst: Return true if the hero can see the monster
bool can_see_monst(AGENT *monster);

//start_run: Set a monster running after something or stop it from running (for when it dies)
void start_run(AGENT *monster);

//chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns true if we want to keep on chasing later. false if we reach the goal.
void chase(AGENT *monster, Coord *chasee_pos);

//roomin: Find what room some coordinates are in. NULL means they aren't in any room.
struct Room *roomin(Coord *cp);

//diag_ok: Check to see if the move is legal if it is diagonal
int diag_ok(Coord *sp, Coord *ep);

//can_see: Returns true if the hero can see a certain coordinate.
int can_see(int y, int x);

//find_dest: find the proper destination for the monster
Coord *find_dest(AGENT *monster);
