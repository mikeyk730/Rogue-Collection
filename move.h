#pragma once

struct Agent;

//diag_ok: Check to see if the move is legal if it is diagonal
int diag_ok(const Coord sp, const Coord ep);

//do_run: Start the hero running
void do_run(byte ch);

//do_move: Check to see that a move is legal.  If it is handle the consequences (fighting, picking up, etc.)
void do_move(int dy, int dx);

//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
void door_open(Room *room);

//be_trapped: The guy stepped on a trap.... Make him pay.
int be_trapped(Coord *tc);

void descend(char *mesg);

//rndmove: Move in a random direction if the monster/person is confused
void rndmove(Agent *who, Coord *newmv);
