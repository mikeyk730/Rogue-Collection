#include "random.h"
#include "room.h"
#include "level.h"
#include "game_state.h"
#include <sstream>

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

bool Room::has_gold() const
{
    return m_gold_val > 0;
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
    m_num_exits = m_flags = 0;
    remove_gold();
}

void Room::remove_gold()
{
    m_gold_val = 0;
}

//todo: shouldn't need to poke into game->
//door: Add a door or possibly a secret door.  Also enters the door in the exits array of the room.
void Room::add_door(Coord p, Level& level)
{
    //Set 1 in 5 doors to be hidden, fewer on the earlier levels
    if (rnd(10) + 1 < game->get_level() && rnd(5) == 0 && !game->wizard().no_hidden_doors())
    {
        level.set_tile(p, (p.y == m_ul_corner.y || p.y == m_ul_corner.y + m_size.y - 1) ? HWALL : VWALL);
        level.unset_flag(p, F_REAL);
    }
    else {
        level.set_tile(p, DOOR);
    }

    int i = m_num_exits++;
    m_exits[i] = p;
}

std::string Room::ToString() const
{
    std::ostringstream ss;
    ss << "Room " << m_index << "(" << m_ul_corner.x << "," << m_ul_corner.y << ") flags: " << std::hex << m_flags;
    return ss.str();
}
