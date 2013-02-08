void clear_level();

byte get_tile(int y, int x);
void set_tile(int y, int x, byte c);

byte get_flags(int y, int x);
void set_flag(int y, int x, byte f);
void unset_flag(int y, int x, byte f);
void copy_flags(int y, int x, byte f);

int get_level();
int next_level();
int prev_level();
int max_level();

int rnd_gold();
