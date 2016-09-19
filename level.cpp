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

int INDEX(int y, int x)
{
  return ((x*(maxrow-1))+y-1);
}

void clear_level()
{
  memset(the_level, ' ', (MAXLINES-3)*MAXCOLS);
  memset(the_flags, F_REAL, (MAXLINES-3)*MAXCOLS);
}

byte get_tile(int y, int x)
{
  return the_level[INDEX(y,x)];
}

void set_tile(int y, int x, byte c)
{
  the_level[INDEX(y,x)] = c;
}

byte get_flags(int y, int x)
{
  return the_flags[INDEX(y,x)];
}

void set_flag(int y, int x, byte f)
{
  the_flags[INDEX(y,x)] |= f;
}

void unset_flag(int y, int x, byte f)
{
  the_flags[INDEX(y,x)] &= ~f;
}

void copy_flags(int y, int x, byte f)
{
  the_flags[INDEX(y,x)] = f;
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
