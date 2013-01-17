//do_run: Start the hero running
do_run(byte ch);

//do_move: Check to see that a move is legal.  If it is handle the consequences (fighting, picking up, etc.)
do_move(int dy, int dx);

//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
door_open(struct room *rp);

//be_trapped: The guy stepped on a trap.... Make him pay.
be_trapped(coord *tc);

descend(char *mesg);

//rndmove: Move in a random direction if the monster/person is confused
rndmove(THING *who, coord *newmv);
