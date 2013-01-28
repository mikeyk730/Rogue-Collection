#include <string.h>

#include "rogue.h"
#include "hero.h"

static char s_name[23];
static int s_purse = 0;
static int s_wizard = 0;
static int s_cheated = 0;

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

void set_wizard(int enable)
{
  s_wizard = enable;
  if (enable)
    s_cheated = TRUE;
}

int is_wizard()
{
  return s_wizard;
}

int did_cheat()
{
  return s_cheated;
}
