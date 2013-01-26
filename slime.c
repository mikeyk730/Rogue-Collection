//Code for handling the various special properties of the slime
//slime.c     1.0     (A.I. Design 1.42)      1/17/85

#include "rogue.h"
#include "slime.h"
#include "misc.h"
#include "thing.h"
#include "curses.h"
#include "io.h"
#include "monsters.h"
#include "chase.h"
#include "main.h"
#include "level.h"

static coord slimy;

//Slime_split: Called when it has been decided that A slime should divide itself

void slime_split(AGENT *tp)
{
  AGENT *nslime;

  if (new_slime(tp)==0 || (nslime = create_agent())==NULL) return;
  msg("The slime divides.  Ick!");
  new_monster(nslime, 'S', &slimy);
  if (cansee(slimy.y, slimy.x))
  {
    nslime->t_oldch = get_tile(slimy.y, slimy.x);
    mvaddch(slimy.y, slimy.x, 'S');
  }
  start_run(&slimy);
}

int new_slime(AGENT *tp)
{
  int y, x, ty, tx, ret;
  AGENT *ntp;
  coord sp;

  ret = 0;
  tp->t_flags |= ISFLY;
  if (plop_monster((ty = tp->t_pos.y), (tx = tp->t_pos.x), &sp)==0)
  {
    //There were no open spaces next to this slime, look for other slimes that might have open spaces next to them.
    for (y = ty-1; y<=ty+1; y++)
      for (x = tx-1; x<=tx+1; x++)
        if (display_character(y, x)=='S' && (ntp = monster_at(y, x)))
        {
          if (ntp->t_flags&ISFLY) continue; //Already done this one
          if (new_slime(ntp)) {y = ty+2; x = tx+2;}
        }
  }
  else {ret = 1; slimy = sp;}
  tp->t_flags &= ~ISFLY;
  return ret;
}

bool plop_monster(int r, int c, coord *cp)
{
  int y, x;
  bool appear = 0;
  byte ch;

  for (y = r-1; y<=r+1; y++)
    for (x = c-1; x<=c+1; x++)
    {
      //Don't put a monster on top of the player.
      if ((y==hero.y && x==hero.x) || offmap(y,x)) continue;
      //Or anything else nasty
      if (step_ok(ch = display_character(y, x)))
      {
        if (ch==SCROLL && find_obj(y, x)->o_which==S_SCARE) continue;
        if (rnd(++appear)==0) {cp->y = y; cp->x = x;}
      }
    }
    return appear;
}
