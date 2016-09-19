#pragma once
#include "room.h"

bool Room::is_flag_set(short flag) const{
    return (flags & flag) != 0;
}

bool Room::is_dark() const{
    return is_flag_set(IS_DARK);
}
bool Room::is_maze() const{
    return is_flag_set(IS_MAZE);
}
bool Room::is_gone() const{
    return is_flag_set(IS_GONE);
}
void Room::set_maze(){
    flags |= IS_MAZE;
}
void Room::set_gone(){
    flags |= IS_GONE;
}
void Room::set_dark(bool enable){
    if (enable)
        flags |= IS_DARK;
    else
        flags &= ~IS_DARK;
}
void Room::reset(){
    goldval = num_exits = flags = 0;
}
