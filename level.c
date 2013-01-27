#include <stdlib.h>

#include "rogue.h"
#include "level.h"
#include "misc.h"

byte the_level[(MAXLINES-3)*MAXCOLS];
byte the_flags[(MAXLINES-3)*MAXCOLS];

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
