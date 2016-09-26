void draw_maze(struct Room *room);

void new_frontier(Coord p);

void add_frnt(Coord p);

//Connect randomly to one of the adjacent points in the spanning tree
void con_frnt();

int maze_at(Coord p);

void splat(Coord p);

int inrange(Coord p);
