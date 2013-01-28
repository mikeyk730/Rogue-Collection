//do_run: Start the hero running
void do_run(byte ch);

//do_move: Check to see that a move is legal.  If it is handle the consequences (fighting, picking up, etc.)
void do_move(int dy, int dx);

//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
void door_open(struct Room *room);

//be_trapped: The guy stepped on a trap.... Make him pay.
int be_trapped(Coord *tc);

void descend(char *mesg);

//rndmove: Move in a random direction if the monster/person is confused
void rndmove(AGENT *who, Coord *newmv);
