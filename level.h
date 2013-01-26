void alloc_level();
void clear_level();

byte get_tile(int y, int x);
void set_tile(int y, int x, byte c);

byte get_flags(int y, int x);
void set_flag(int y, int x, byte f);
void unset_flag(int y, int x, byte f);
void copy_flags(int y, int x, byte f);
