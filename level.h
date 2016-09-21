void clear_level();

namespace Level {
    byte get_tile(Coord p);
    void set_tile(Coord p, byte c);

    byte get_flags(Coord p);
    void set_flag(Coord p, byte f);
    void unset_flag(Coord p, byte f);
    void copy_flags(Coord p, byte f);

    bool is_passage(Coord p);
    bool is_maze(Coord p);
    bool is_real(Coord p);
    int get_passage_num(Coord p);
    int get_trap_type(Coord p);
}


int get_level();
int next_level();
int prev_level();
int max_level();

int rnd_gold();
