//do_passages: Draw all the passages on a level.
void do_passages();

//door: Add a door or possibly a secret door.  Also enters the door in the exits array of the room.
void door(struct room *rm, coord *cp);

void psplat(shint y, shint x);

//passnum: Assign a number to each passageway
void passnum();

//numpass: Number a passageway square and its brethren
void numpass(int y, int x);
