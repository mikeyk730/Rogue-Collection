#include <string.h>

#include "hero.h"

static char s_name[23];
static int s_purse = 0;

const char* get_name()
{
  return s_name;
}

void set_name(const char* name)
{
  strcpy(s_name, name);
}

int get_purse()
{
  return s_purse;
}

void adjust_purse(int delta)
{
  s_purse += delta;
  if (s_purse < 0) 
    s_purse = 0;
}
