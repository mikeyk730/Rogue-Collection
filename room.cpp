#pragma once
#include "room.h"

bool Room::is_flag_set(short flag) const {
    return (m_flags & flag) != 0;
}

bool Room::is_dark() const {
    return is_flag_set(IS_DARK);
}
bool Room::is_maze() const {
    return is_flag_set(IS_MAZE);
}
bool Room::is_gone() const {
    return is_flag_set(IS_GONE);
}
void Room::set_maze() {
    m_flags |= IS_MAZE;
}
void Room::set_gone() {
    m_flags |= IS_GONE;
}
void Room::set_dark(bool enable) {
    if (enable)
        m_flags |= IS_DARK;
    else
        m_flags &= ~IS_DARK;
}
void Room::reset() {
    m_gold_val = m_num_exits = m_flags = 0;
}
