//do_rooms: Create rooms and corridors with a connectivity graph
void do_rooms();

//draw_room: Draw a box around a room and lay down the floor
void draw_room(struct room *rp);

//vert: Draw a vertical line
void vert(struct room *rp, int startx);

//horiz: Draw a horizontal line
void horiz(struct room *rp, int starty);

//rnd_pos: Pick a random spot in a room
void rnd_pos(struct room *rp, coord *cp);

//enter_room: Code that is executed whenever you appear in a room
void enter_room(coord *cp);

//leave_room: Code for when we exit a room
void leave_room(coord *cp);
