//do_passages: Draw all the passages on a m_level.
void do_passages();

//door: Add a door or possibly a secret door.  Also enters the door in the exits array of the m_room.
void add_door(struct Room *rm, Coord *cp);

void psplat(Coord p);

//passnum: Assign a number to each passageway
void passnum();

//numpass: Number a passageway square and its brethren
void numpass(Coord p);

//add_pass: Add the passages to the current window (wizard command)
void add_pass();

