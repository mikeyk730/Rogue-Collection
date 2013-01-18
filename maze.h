void draw_maze(struct room *rp);

void new_frontier(int y, int x);

void add_frnt(int y, int x);

//Connect randomly to one of the adjacent points in the spanning tree
void con_frnt();

int maze_at(int y, int x);

void splat(int y, int x);

int inrange(int y, int x);
