#include "rogue.h"

//flags for rooms
const short IS_DARK = 0x0001; //room is dark
const short IS_GONE = 0x0002; //room is gone (a corridor)
const short IS_MAZE = 0x0004; //room is a maze

//Room structure
struct Room
{
    int index;
    Coord pos;       //Upper left corner
    Coord size;      //Size of room
    Coord gold;      //Where the gold is
    int gold_val;     //How much the gold is worth
    short flags;     //Info about the room
    int num_exits;   //Number of exits
    Coord exits[12]; //Where the exits are

    bool is_flag_set(short flag) const;

    bool is_dark() const;
    bool is_maze() const;
    bool is_gone() const;
    void set_maze();
    void set_gone();
    void set_dark(bool enable);
    void reset();
};
