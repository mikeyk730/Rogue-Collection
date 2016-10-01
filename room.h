#include "rogue.h"

//flags for rooms
const short IS_DARK = 0x0001; //room is dark
const short IS_GONE = 0x0002; //room is gone (a corridor)
const short IS_MAZE = 0x0004; //room is a maze

//Room structure
struct Room
{
    int index;
    Coord m_ul_corner;      //Upper left corner
    Coord m_size;           //Size of room
    Coord m_gold_position;  //Where the gold is
    int m_gold_val;         //How much the gold is worth
    short m_flags;          //Info about the room
    int m_num_exits;        //Number of exits
    Coord m_exits[12];      //Where the exits are

    bool is_flag_set(short flag) const;

    bool is_dark() const;
    bool is_maze() const;
    bool is_gone() const;
    void set_maze();
    void set_gone();
    void set_dark(bool enable);
    void reset();

    //door: Add a door or possibly a secret door.  Also enters the door in the exits array of the room.
    void add_door(Coord cp);

};
