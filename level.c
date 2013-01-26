#include <stdlib.h>
#include <wchar.h>

#include "rogue.h"
#include "level.h"
#include "misc.h"

byte *the_level = NULL;
byte *the_flags = NULL;

void alloc_level()
{
  the_level = malloc((MAXLINES-3)*MAXCOLS);
  the_flags = malloc((MAXLINES-3)*MAXCOLS);
}

void clear_level()
{
  wsetmem((wchar_t*)the_level, ((MAXLINES-3)*MAXCOLS)>>1, L'\x2020');
  setmem(the_flags, (MAXLINES-3)*MAXCOLS, F_REAL);
}

byte chat(int y, int x)
{
  return the_level[INDEX(y,x)];
}

void set_chat(int y, int x, byte c)
{
  the_level[INDEX(y,x)] = c;
}

byte flat(int y, int x)
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
