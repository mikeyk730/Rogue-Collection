/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
 * Copyright (C) 1985 by Appel, Jacobson, Hamey, and Mauldin.
 *
 * This file is part of Rog-O-Matic.
 *
 * Rog-O-Matic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rog-O-Matic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rog-O-Matic.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * survival.c:
 *
 * This file contains all of the "Run Away" code.
 * Well, almost all of the run away code.
 * At least I think it has something to do with running away.
 *
 * 7-Mar-87  Michael Mauldin (mlm) at Carnegie-Mellon University
 * Modified markcycles() to check for choke points in corridors,
 * as possible RUNOK squares, in addition to doors.  This fixes the
 * bug where a cycle through corridors was not noticed because
 * there was no door there.
 */

# include <stdio.h>
# include <ctype.h>
# include <curses.h>
# include "types.h"
# include "globals.h"

# define SO	  1
# define SE	  0

# define highlight(rowcol,stand)		\
  if (print || debug (D_SCREEN))		\
  { at((rowcol)/MAXCOLS,(rowcol)%MAXCOLS);		\
    if (stand) standout ();			\
    printw("%c",screen[0][rowcol]);		\
    if (stand) standend ();			\
    refresh (); }

/*
 * markcycles: evokes fond memories of an earlier time, when Andrew
 * was a hacker who just patched things together until they worked,
 * and used the SHIFT-STOP key to compile things, rather than thinking.
 *
 * markcycles does a depth-first-search of the squares to find loops in
 * the rooms.  All doors on loops are marked RUNOK, and are used by the
 * runaway code.
 */

int markcycles (print)
{
  short mark[1920];
  struct {short where,door,dirs;} st[1000];
  register int sp,newsquare; int *Scr; int whichdir; int D;

  if (!new_mark) return (0);

  Scr=scrmap[0];

  markchokepts ();

  { register int count=1920; register short *m=mark; while(count--) *m++=0;}
  sp=1; st[1].where=atrow*MAXCOLS+atcol; st[1].dirs=1; st[1].door=0;

  for (D = 0; D < 8; D += 2) {
    if ((Scr[newsquare = (st[1].where+deltrc[D^4])]) & CANGO) {
      if (mark[newsquare]) {
        int stop, i;

        if (mark[newsquare] < sp)
          for (stop = st[mark[newsquare]].door,
               i = (Scr[st[sp].where] & CHOKE) ? sp : st[sp].door;
               i !=  stop;
               i =st[i].door) {
            Scr[st[i].where] |= RUNOK;
            highlight (st[i].where, SO)
          }
      }
      else {
        sp++; mark[newsquare] = sp;
        highlight (newsquare, SO)
        st[sp].where=newsquare;
        st[sp].dirs=1; st[1].dirs= -1;
        st[sp].door = (Scr[st[sp-1].where]&CHOKE) ? sp-1 : st[sp-1].door;
      }
    }

    while (sp > 1) {
      if ((whichdir=(st[sp].dirs++)<<1)<8) {
        /* whichdir is 6,2, or 4. */
        if ((Scr[newsquare= (st[sp].where+deltrc[(whichdir+D)&7])])&CANGO) {
          if (mark[newsquare]) {
            register int stop,i;

            if (mark[newsquare]<sp) {
              for (stop=st[mark[newsquare]].door,
                   i=(Scr[st[sp].where]&CHOKE)?sp:st[sp].door;
                   i!=stop;
                   i=st[i].door) {
                Scr[st[i].where] |= RUNOK;
                highlight (st[i].where, SO)
              }
            }
          }
          else {
            sp++; mark[newsquare]=sp;
            highlight (newsquare, SO)
            st[sp].where=newsquare;
            st[sp].dirs=1; D += whichdir+4;
            st[sp].door = (Scr[st[sp-1].where]&CHOKE) ? sp-1 : st[sp-1].door;
          }
        }
      }
      else {
        if (! (Scr[st[sp].where] & RUNOK)) highlight (st[sp].where, SE)
          sp--;

        D -= 4+((st[sp].dirs-1)<<1);
      }
    }
  }

  highlight (st[1].where, SE)

  new_mark = 0;
  return (1);
}

/*
 * markchokepts: Mark places to check for cycles.  A choke point
 * is any door or any hall with only two neighbors.
 *
 * Added: 3/7/87 by mlm
 */

void markchokepts ()
{
  register int *Scr, *ScrEnd;

  for (Scr = scrmap[0], ScrEnd = &Scr[1920]; Scr<ScrEnd; Scr++) {
    if (*Scr & DOOR) *Scr |= CHOKE;
    else if (*Scr & HALL) {
      register int nbrs = 0, k;

      for (k=0; k<8; k++)
        { if (Scr[deltrc[k]] & CANGO) nbrs++; }

      if (nbrs < 4 ||
          ! (Scr[  1]      & Scr[-(MAXCOLS-1)] & Scr[-MAXCOLS] & CANGO ||
             Scr[-MAXCOLS] & Scr[-(MAXCOLS+1)] & Scr[ -1]      & CANGO ||
             Scr[ -1]      & Scr[ (MAXCOLS-1)] & Scr[ MAXCOLS] & CANGO ||
             Scr[ MAXCOLS] & Scr[ (MAXCOLS+1)] & Scr[  1]      & CANGO)) {
        *Scr |= CHOKE;

        if (debug (D_SCREEN)) {
          register int rowcol = Scr - scrmap[0];
          standout ();
          mvprintw (rowcol/MAXCOLS, rowcol%MAXCOLS, "C");
          standend ();
        }
      }
    }
  }
}

/*
 * Runaway: Panic!
 */

int runaway ()
{
  if (on (SCAREM)) {
    dwait (D_BATTLE, "Not running, on scare monster scroll!");
    return (0);
  }

  dwait (D_BATTLE | D_SEARCH, "Run away!!!!");

  if (on (STAIRS) && !floating)		/* Go up or down */
    return (goupstairs (RUNNING) || godownstairs (RUNNING));

  if (canrun ())		/* If canrun finds a move, use it */
    return (followmap (RUNAWAY));

  return (0);			/* Cant run away */
}

/*
 * Canrun: set up a move which will get us away from danger.
 */

int canrun ()
{
  int result, oldcomp = compression;

  if (on (STAIRS)) return (1);		/* Can run down stairs */

  compression = 0;			/* Be tense when fleeing */
  result = (findmove (RUNAWAY, runinit, runvalue, REEVAL) ||
            findmove (EXPLORERUN, expruninit, exprunvalue, REEVAL));

  compression = oldcomp;
  return (result);
}

/*
 * unpin:
 *
 *	"When you're stuck and wriggling on a pin,
 *	 When you're pinned and wriggling on a wall..."
 *
 *		"The Love Song of J. Alfred Prufrock", T.S. Eliot
 */

int unpin ()
{
  int result, oldcomp = compression;
  int unpininit (), runvalue (), expunpininit (),
      exprunvalue (), expunpinvalue ();

  if (on (SCAREM)) {
    dwait (D_BATTLE, "Not unpinning, on scare monster scroll!");
    return (0);
  }

  if (on (STAIRS) && !floating) {
    if (!goupstairs (RUNNING)) godownstairs (RUNNING);

    return (1);
  }

  dwait (D_BATTLE, "Pinned!!!!");

  /* currentrectangle ();   // always done after each move of the rogue // */

  compression = 0;	/* Be tense when fleeing */
  result = (makemove (UNPIN, unpininit, runvalue, REEVAL) ||
            makemove (UNPINEXP, expunpininit, expunpinvalue, REEVAL));

  compression = oldcomp;
  return (result);
}

/*
 * backtodoor: Useful when being ganged up on. Back into the nearest
 *             door.
 */

int backtodoor (dist)
int dist;
{
  static int lastcall= -10, stillcount=0, notmoving=0, closest=99;

  /*
   * Keep track of the opponents distance.  If they stop advancing on us,
   * disable the rule for 10 turns.
   */

  if (turns-lastcall > 20)
    { notmoving=0; closest=99; stillcount=0; }
  else if (dist < closest)
    { closest=dist; stillcount=0; }
  else if (++stillcount > 5)
    { notmoving++; }

  lastcall = turns;

  /*
   * Now check whether we try to move back to the door
   */

  if (notmoving)
    dwait (D_BATTLE, "backtodoor: monsters not moving");

  else if (on (SCAREM))
    dwait (D_BATTLE, "Not backing up, on scare monster scroll!");

  else if (dist > 0 && (on (DOOR) || nextto (DOOR, atrow, atcol)))
    dwait (D_BATTLE, "backtodoor: next to door, have time");

  else if (makemove (RUNTODOOR, rundoorinit, rundoorvalue, REEVAL))
    { dwait (D_BATTLE, "Back to the door..."); return (1); }

  return (0);
}
