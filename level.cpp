#include <stdlib.h>

#include "rogue.h"
#include "level.h"
#include "misc.h"
#include "io.h"
#include "main.h"
#include "room.h"

byte the_level[(MAXLINES-3)*MAXCOLS];
byte the_flags[(MAXLINES-3)*MAXCOLS];

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

void clear_level()
{
  memset(the_level, ' ', (MAXLINES-3)*MAXCOLS);
  memset(the_flags, F_REAL, (MAXLINES-3)*MAXCOLS);
}

namespace Level {
    int INDEX(Coord p)
    {
        return ((p.x*(maxrow - 1)) + p.y - 1);
    }

    byte get_tile(Coord p)
    {
        return the_level[INDEX(p)];
    }

    void set_tile(Coord p, byte c)
    {
        the_level[INDEX(p)] = c;
    }

    byte get_flags(Coord p)
    {
        return the_flags[INDEX(p)];
    }

    void set_flag(Coord p, byte f)
    {
        the_flags[INDEX(p)] |= f;
    }

    void unset_flag(Coord p, byte f)
    {
        the_flags[INDEX(p)] &= ~f;
    }

    void copy_flags(Coord p, byte f)
    {
        the_flags[INDEX(p)] = f;
    }
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
