//do_passages: Draw all the passages on a level.
do_passages();

//door: Add a door or possibly a secret door.  Also enters the door in the exits array of the room.
door(struct room *rm, coord *cp);

void psplat(shint y, shint x);

//passnum: Assign a number to each passageway
passnum();

//numpass: Number a passageway square and its brethren
void numpass(int y, int x);
