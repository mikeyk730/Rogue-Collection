//routines for writing a fake dos

#include <ctype.h>

#include "rogue.h"
#include "game_state.h"
#include "output_interface.h"
#include "fakedos.h"
#include "mach_dep.h"
#include "io.h"
#include "strings.h"

int dodos(char *com);

void fakedos()
{
  char comline[132];
  char *comhead;

  game->screen().wdump();
  game->screen().clear();
  game->screen().move(0, 0);
  game->screen().cursor(true);
  do
  {
    memset(comline, 0, sizeof(comline));
    game->screen().printw("\nC:\\>");
    getinfo(comline, 130);
    comhead = stpblk(comline);
    endblk(comhead);
  } while (dodos(comhead));
  game->screen().cursor(false);
  game->screen().wrestor();
}

//execute a dos like command
int dodos(char *com)
{
  if (strcmp(com, "rogue")==0) return 0;
  game->screen().printw("\nBad command or file name\n");
  return 1;
}
