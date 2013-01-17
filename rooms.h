//do_rooms: Create rooms and corridors with a connectivity graph
do_rooms();

//draw_room: Draw a box around a room and lay down the floor
draw_room(struct room *rp);

//vert: Draw a vertical line
vert(struct room *rp, int startx);

//horiz: Draw a horizontal line
horiz(struct room *rp, int starty);

//rnd_pos: Pick a random spot in a room
rnd_pos(struct room *rp, coord *cp);

//enter_room: Code that is executed whenever you appear in a room
enter_room(coord *cp);

//leave_room: Code for when we exit a room
leave_room(coord *cp);
