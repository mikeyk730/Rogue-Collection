#include <stdlib.h>
#include <wchar.h>

#include "rogue.h"
#include "level.h"
#include "misc.h"

byte *the_level = NULL;

void alloc_level()
{
  the_level = malloc((MAXLINES-3)*MAXCOLS);
}

void clear_level()
{
  wsetmem((wchar_t*)the_level, ((MAXLINES-3)*MAXCOLS)>>1, L'\x2020');
}

byte chat(int y, int x)
{
  return the_level[INDEX(y,x)];
}

void set_chat(int y, int x, byte c)
{
  the_level[INDEX(y,x)] = c;
}