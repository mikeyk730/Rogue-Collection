#include <list>

struct Item;
struct Agent;

struct Level {
    void new_level(int do_implode);

    //put_things: Put potions and scrolls on this level
    void put_things();

    //treas_room: Add a treasure room
    void treas_room();

    void clear_level();

    byte get_tile(Coord p);
    byte get_tile_or_monster(Coord p);
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

    Room* get_passage(Coord pos);

    //monster_at: returns pointer to monster at coordinate. if no monster there return NULL
    Agent *monster_at(Coord p);

    void draw_char(Coord p);


    std::list<Item*> items; //List of objects on this level
    std::list<Agent*> monsters; //List of monsters on the level
private:
    byte the_level[(MAXLINES - 3)*MAXCOLS];
    byte the_flags[(MAXLINES - 3)*MAXCOLS];
};

int get_level();
int next_level();
int prev_level();
int max_level();

int rnd_gold();
