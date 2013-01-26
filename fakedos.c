//routines for writing a fake dos

#include <ctype.h>

#include "rogue.h"
#include "curses.h"
#include "fakedos.h"
#include "mach_dep.h"
#include "io.h"
#include "strings.h"

void fakedos()
{
  char comline[132];
  char *comhead;

  wdump();
  clear();
  move(0, 0);
  cursor(TRUE);
  do
  {
    memset(comline, 0, sizeof(comline));
    printw("\nC:\\>");
    getinfo(comline, 130);
    comhead = stpblk(comline);
    endblk(comhead);
  } while (dodos(comhead));
  cursor(FALSE);
  wrestor();
}

//execute a dos like command
int dodos(char *com)
{
  if (strcmp(com, "rogue")==0) return 0;
  printw("\nBad command or file name\n");
  return 1;
}
