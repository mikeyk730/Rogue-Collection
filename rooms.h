//do_rooms: Create rooms and corridors with a connectivity graph
void do_rooms();

//draw_room: Draw a box around a room and lay down the floor
void draw_room(struct Room *room);

//vert: Draw a vertical line
void vert(struct Room *room, int startx);

//horiz: Draw a horizontal line
void horiz(struct Room *room, int starty);

//rnd_pos: Pick a random spot in a room
void rnd_pos(struct Room *room, Coord* cp);

//enter_room: Code that is executed whenever you appear in a room
void enter_room(Coord cp);

//leave_room: Code for when we exit a room
void leave_room(Coord cp);

Room* rnd_room();

//get_room_from_position: Find what room some coordinates are in. NULL means they aren't in any room.
Room* get_room_from_position(Coord cp);

void find_empty_location(Coord* c, bool consider_monsters);