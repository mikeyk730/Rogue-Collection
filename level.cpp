#include <stdlib.h>

#include "rogue.h"
#include "game_state.h"
#include "level.h"
#include "misc.h"
#include "io.h"
#include "main.h"
#include "room.h"
#include "agent.h"

//One for each passage
struct Room passages[MAXPASS] =
{
  { 0, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 1, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 2, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 3, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 4, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 5, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 6, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 7, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 8, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 9, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 10, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 },
  { 11, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE|IS_DARK, 0, 0 }
};

int s_level = 1;
int s_max_level = 1;

void Level::clear_level()
{
  memset(the_level, ' ', (MAXLINES-3)*MAXCOLS);
  memset(the_flags, F_REAL, (MAXLINES-3)*MAXCOLS);
}

    int INDEX(Coord p)
    {
        return ((p.x*(maxrow - 1)) + p.y - 1);
    }

    byte Level::get_tile(Coord p)
    {
        return the_level[INDEX(p)];
    }

    byte Level::get_tile_or_monster(Coord p)
    {
        Agent* monster = monster_at(p);
        if (monster)
            return monster->disguise;
        return get_tile(p);
    }


    void Level::set_tile(Coord p, byte c)
    {
        the_level[INDEX(p)] = c;
    }

    byte Level::get_flags(Coord p)
    {
        return the_flags[INDEX(p)];
    }

    void Level::set_flag(Coord p, byte f)
    {
        the_flags[INDEX(p)] |= f;
    }

    void Level::unset_flag(Coord p, byte f)
    {
        the_flags[INDEX(p)] &= ~f;
    }

    void Level::copy_flags(Coord p, byte f)
    {
        the_flags[INDEX(p)] = f;
    }

    bool Level::is_passage(Coord p)
    {
        return (get_flags(p) & F_PASS) != 0;
    }

    bool Level::is_maze(Coord p)
    {
        return (get_flags(p) & F_MAZE) != 0;
    }

    bool Level::is_real(Coord p)
    {
        return (get_flags(p) & F_REAL) != 0;
    }

    int Level::get_passage_num(Coord p)
    {
        return get_flags(p) & F_PNUM;
    }

    int Level::get_trap_type(Coord p)
    {
        return get_flags(p) & F_TMASK;
    }

    //monster_at: returns pointer to monster at coordinate. if no monster there return NULL
    Agent* Level::monster_at(Coord p)
    {
        Agent *monster;
        for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it) {
            monster = *it;
            if (monster->pos.x == p.x && monster->pos.y == p.y)
                return monster;
        }
        return NULL;
    }


int get_level()
{
  return s_level;
}

int next_level()
{
  ++s_level;
  if (s_level > s_max_level) 
    s_max_level = s_level;
  return s_level;
}

int prev_level()
{
  return --s_level;
}

int max_level()
{
  return s_max_level;
}

int rnd_gold()
{
  return (rnd(50+10*s_level)+2);
}
