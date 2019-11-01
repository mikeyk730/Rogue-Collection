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
 * search.c:
 *
 * This file contains the very basic search mechanisms for exploration etc.
 */

# include <stdio.h>
# include <curses.h>
# include "types.h"
# include "globals.h"

# define QSIZE (4000)

# define QUEUEBREAK  (111)
# define FROM         (20)
# define UNREACHABLE  (12)
# define NOTTRIED     (11)
# define TARGET       (10)

static int moveavd[MAXROWS][MAXCOLS], moveval[MAXROWS][MAXCOLS], movecont[MAXROWS][MAXCOLS],
       movedepth[MAXROWS][MAXCOLS];
static char mvdir[MAXROWS][MAXCOLS];
static int mvtype=0;
static int didinit=0;

/*
 * makemove: repeat move from here towards some sort of target.
 * Modified to use findmove.			5/13	MLM
 */

int makemove (movetype, evalinit, evaluate, reevaluate)
int movetype, reevaluate;
evalinit_ptr evalinit;
evaluate_ptr evaluate;
{
  if (findmove (movetype, evalinit, evaluate, reevaluate))
    return (followmap (movetype));

  return (0);
}

/*
 * findmove: search for a move of type movetype.  The move map is left in
 *           the correct state for validatemap or followmap to work.	MLM
 */

int findmove (movetype, evalinit, evaluate, reevaluate)
int movetype, reevaluate;
evalinit_ptr evalinit;
evaluate_ptr evaluate;
{
  int result;

  didinit = ontarget = 0;

  if (!reevaluate) {	/* First try to reuse the movement map */
    result = validatemap (movetype, evalinit, evaluate);

    if (result == 1) return (1);	/* Success */

    if (result == 2) return (0);	/* Evalinit failed, no move */
  }

  /* Must rebuild the movement map */
  mvtype = 0;	/* Will become 'if (mvtype==movetype) movetype=0;' */

  dwait (D_SEARCH, "Findmove: computing new search path.");

  /* currentrectangle(); */     /* always done after each move of the rogue */

  searchstartr = atrow; searchstartc = atcol;

  if (!(*evalinit)())    /* Compute evalinit from current location */
    { dwait (D_SEARCH, "Findmove: evalinit failed."); return (0); }

  if (!searchfrom (atrow, atcol, evaluate, mvdir, &targetrow, &targetcol))
    { return (0); }	/* move failed */

  if (targetrow == atrow && targetcol == atcol)
    { ontarget = 1; return (0); }

  /* <<copy the newly created map to save*[][]>> */
  mvtype = movetype;	/* mvtype will be the type of saved map */

  return (1);
}

/*
 * followmap: Assuming that the mvdir map is correct, send a movement
 *            command following the map (possibly searching first).
 *
 *	<<Must be changed to use the saved map, when that code is added>>
 *
 * May 13, MLM
 */

int followmap (movetype)
register int movetype;
{
  register int dir, dr, dc, r, c;
  int timemode, searchit, count=1;

  dir=mvdir[atrow][atcol]-FROM; dr=deltr[dir]; dc=deltc[dir];

  if (dir > 7 || dir < 0) {
    if (dir < 0) {
      dwait (D_ERROR, "Followmap: direction invalid!  < 0  dir %d,  atr %d atc %d",
                   dir, atrow, atcol);
      return (0);			      /* Something Broke */
    }
    else if (dir >= TARGET) {
      dir = dir - TARGET;
      if (dir > 7) {
        dwait (D_ERROR, "Followmap: adjusted direction still invalid!  dir %d,  atr %d atc %d",
                   dir, atrow, atcol);
        return (0);			      /* Something Broke still */
      }
    }
  }

  r=atrow+dr; c=atcol+dc;		/* Save next square in registers */

  /* If exploring and are moving to a new hall square, use fmove */
  if (movetype == EXPLORE &&
      onrc (HALL|BEEN, targetrow, targetcol) != (HALL|BEEN) &&
      onrc (HALL,r,c) &&
      !beingstalked)			/* Feb 10, 1985 - mlm */
    { fmove (dir); return (1); }

  /* Timemode tells why we are moving this way, T_RUNNING ==> no search */
  timemode = (movetype == GOTOMOVE)    ? T_MOVING :
             (movetype == EXPLORE)     ? T_EXPLORING :
             (movetype == EXPLOREROOM) ? T_EXPLORING :
             (movetype == FINDROOM)    ? T_EXPLORING :
             (movetype == EXPLORERUN)  ? T_RUNNING :
             (movetype == RUNTODOOR)   ? T_RUNNING :
             (movetype == RUNAWAY)     ? T_RUNNING :
             (movetype == UNPIN)       ? T_RUNNING :
             (movetype == UNPINEXP)    ? T_RUNNING :
             (movetype == RUNAWAY)     ? T_RUNNING :
             (movetype == RUNDOWN)     ? T_RUNNING :
             (movetype == ATTACKSLEEP) ? T_FIGHTING :  T_MOVING;

  /* How many times do we wish to search each square before moving? */
  /* Search up to k times if 2 or more foods and deeper than level 6 */
  searchit = max (0, min (k_srch/20, min (larder - 1, Level - 6)));

  /* Can we move more than one square at a time? Dont count scare monsters! */
  if (compression) {
    while (mvdir[r][c]-FROM==dir &&
           (onrc (SAFE|SCAREM, r+=dr, c+=dc) == SAFE || !searchit))
      count++;
  }

  /* Maybe search unsafe square before moving onto it */
  if (timemode != T_RUNNING && !onrc (SAFE, atrow+dr, atcol+dc) &&
      timessearched[atrow+dr][atcol+dc] < searchit)
    { command (T_SEARCHING, "s"); return (1); }

  /* Maybe take armor off before stepping on rust trap */
  if (timemode != T_RUNNING && onrc (WATERAP, atrow+dr, atcol+dc) &&
      currentarmor != NONE && willrust (currentarmor) && takeoff ())
    { rmove (1, dir, timemode); return (1); }

  /* If we are about to step onto a scare monster scroll, use the 'm' cmd */
  if (can_move_without_pickup() && onrc (SCAREM, atrow+dr, atcol+dc))
    { mmove (dir, timemode); return (1); }

  /* Send the movement command and return success */
  rmove (count, dir, timemode); return (1);
}

/*
 * validatemap: If we have a stored move, make it and return true.
 *
 *	<<Must be changed to use the saved map, when that code is added>>
 *
 * Called only by findmove.	MLM
 */

int validatemap (movetype, evalinit, evaluate)
int movetype;
evalinit_ptr evalinit;
evaluate_ptr evaluate;
{
  register int thedir, dir, r, c;
  int val, avd, cont;

  dwait (D_CONTROL | D_SEARCH, "Validatemap: type %d", movetype);

  if (mvtype != movetype) {
    dwait (D_SEARCH, "Validatemap: move type mismatch, map invalid.");
    return (0);
  }

  thedir = mvdir[atrow][atcol] - FROM;

  if (thedir > 7 || thedir < 0) {
    dwait (D_SEARCH, "Validatemap: direction in map invalid.");
    return (0);  /* Something Broke */
  }

  /*
   * Check that the planned path is still valid.  This is done by
   * proceeding along it and checking that the value and avoidance
   * returned from the evaluation function are the same as
   * when the search was first performed.  The initialisation function
   * is re-performed and then the evaluation function done.
   */

  if (!didinit && !(*evalinit)()) {
    dwait (D_SEARCH, "Validatemap: evalinit failed.");
    return (2);  /* evalinit failed */
  }

  didinit=1;

  r=atrow; c=atcol;

  while (1) {
    val = avd = cont = 0;

    if (!(*evaluate)(r, c, movedepth[r][c], &val, &avd, &cont)) {
      dwait (D_SEARCH, "Validatemap: evaluate failed.");
      return (0);
    }

    if (!onrc (CANGO, r, c) ||
        avd!=moveavd[r][c] || val!=moveval[r][c] || cont!=movecont[r][c]) {
      dwait (D_SEARCH, "Validatemap: map invalidated.");
      return (0);
    }

    if ((dir=mvdir[r][c]-FROM) == TARGET) {
      dwait (D_SEARCH, "Validatemap: existing map validated.");
      break;
    }

    if (dir < 0 || dir > 7) {
      dwait (D_SEARCH, "Validatemap: direction in map invalid.");
      return (0);
    }

    r += deltr[dir];  c += deltc[dir];
  }

  return (1);
}

/*
 * cancelmove: Invalidate all stored moves of a particular type.
 */

void cancelmove (movetype)
int movetype;
{
  if (movetype == mvtype) mvtype = 0;
}

/*
 * setnewgoal: Invalidate all stored moves.
 */

void setnewgoal ()
{
  mvtype = 0;
  goalr = goalc = NONE;
}

/*
 * searchfrom: By means of breadth first search, find a path
 * from the given row and column to a target.  This is done by using
 * searchto and then reversing the path to the row, col from the selected
 * target.  Note that this means that the resultant direction map can
 * only be re-used if the new row, col is on the existing path.  The
 * reversed path consists of directions offset by FROM.
 * arguments and results otherwise the same as searchto.	LGCH
 */

int searchfrom (row, col, evaluate, dir, trow, tcol)
int row, col, *trow, *tcol;
evaluate_ptr evaluate;
char dir[MAXROWS][MAXCOLS];
{
  register int r, c, sdir, tempdir;

  if (!searchto (row, col, evaluate, dir, trow, tcol)) {
    return (0);
  }

  for (r = *trow, c = *tcol, sdir = FROM+TARGET; ; ) {
    tempdir = dir[r][c];
    dir[r][c] = sdir;

    if (debug (D_SCREEN | D_INFORM | D_SEARCH))
      { at (r, c);  printw ("%c", ">/^\\</v\\  ~"[sdir-FROM]);}

    sdir = (tempdir + 4) % 8 + FROM;  /* reverse direction and offset */

    if (tempdir == TARGET) break;

    r += deltr[tempdir];  c += deltc[tempdir];
  }

  dwait (D_SEARCH, "Searchfrom wins.");
  return (1);
}

/*
 * searchto: By means of a breadth first search, find a path to the
 * given row and column from a target.  A target is defined as a
 * location which has +ve value returned by the evaluation function and
 * for which the avoidance value has been decremented to zero. The most
 * valuable target found in the first successful iteration of the
 * search, is selected. (i.e. the most valuable square at the lowest
 * level of the search).  Returns dir the direction map of paths to
 * row,col from target Also returns trow, tcol the position of the
 * selected target (NOTE: To use this search directly, e.g. to find
 * paths to a single actual target such as the staircase, the
 * evaluation function should give zero value to everything except the
 * current Rog-O-Matic location To re-use the results of a search,
 * ensure that dir[row][col] is still set to TARGET and check that a
 * valid direction exists at the target position.)
 *
 * The search prefers horizontal movements to vertical movements, and
 * prefers moves onto SAFE squares to moves onto other squares.	       LGCH
 */

/*
 * Since this code is the single most time consuming subroutine, I am
 * attempting to hack it into a faster form. 			11/6/82 MLM
 */

int searchto (row, col, evaluate, dir, trow, tcol)
int row, col, *trow, *tcol;
evaluate_ptr evaluate;
char dir[MAXROWS][MAXCOLS];
{
  int searchcontinue = 10000000, type, havetarget=0, depth=0;
  register int r, c, nr, nc;
  register int k;
  char begin[QSIZE], *end, *head, *tail;
  int saveavd[MAXROWS][MAXCOLS], val, avd, cont;
  int any;
  static int sdirect[8] = {4, 6, 0, 2, 5, 7, 1, 3},
             sdeltr[8]  = {0,-1, 0, 1,-1,-1, 1, 1},
             sdeltc[8]  = {1, 0,-1, 0, 1,-1,-1, 1};

  head = tail = begin;
  end = begin + QSIZE;

  for (c = 23*80; c--; ) dir[0][c] = NOTTRIED;		/* MLM */

  for (c = 80; c--; ) dir[0][c] = 0;			/* MLM */

  *(tail++) = row;  *(tail++) = col;
  *(tail++) = QUEUEBREAK;  *(tail++) = QUEUEBREAK;
  dir[row][col] = TARGET;  moveval[row][col] = NONE;
  any = 1;

  while (1) {
    /* Process the next queued square. */
    r = *(head++);  c = *(head++);

    if (head == end) head = begin;  /* wrap-around queue */

    if (r==QUEUEBREAK) {
      /* If we have completed an evaluation loop */
      if (searchcontinue <= 0 || !any) {
        if (havetarget) dwait (D_SEARCH, "Searchto wins.");
        else dwait (D_SEARCH, "Searchto fails.");

        return (havetarget);  /* have found somewhere to go */
      }

      searchcontinue--;   depth++;

      /* ----------------------------------------------------------------
      if (debug (D_SCREEN))
        dwait (D_SEARCH, "Searchto: at queue break, cont=%d, havetarget=%d",
         searchcontinue, havetarget);
      ---------------------------------------------------------------- */

      any = 0;    /* None found in queue this time round */

      *(tail++) = QUEUEBREAK;  *(tail++) = QUEUEBREAK;

      if (tail == end) tail = begin;

      continue;
    }

    any = 1;   /* Something in queue */

    if (moveval[r][c] == NONE) {
      /* unevaluated: evaluate it */
      val = avd = cont = 0;

      if ((*evaluate)(r,c,depth,&val,&avd,&cont)) { /* Evaluate it. */
        movedepth[r][c] = depth;
        moveavd[r][c] = avd;
        moveval[r][c] = val;
        movecont[r][c] = cont;

        if (avd >= ROGINFINITY) {
          /* Infinite avoidance */
          dir[r][c]=UNREACHABLE;  /* we cant get here */
          continue;	/* discard the square from consideration. */
        }
        else {
          saveavd[r][c]=avd;
        }
      }
      else {	/* If evaluate fails, forget it for now. */
        dwait (D_SEARCH, "Searchto: evaluate failed.");
        continue;
      }
    }

    if (saveavd[r][c]) {
      /* If to be avoided, leave in queue for a while */
      *(tail++) = r;  *(tail++) = c;   --(saveavd[r][c]);  /* Dec avoidance */

      if (tail == end) tail = begin;

      continue;
    }

    if (moveval[r][c] > havetarget) {
      /* It becomes the target if it has value bigger than the best found
      so far, and if it has a non-zero value.
       */

      if (debug (D_SCREEN | D_SEARCH | D_INFORM)) {
        mvprintw (r, c, "=");
        dwait (D_SEARCH, "Searchto: target value %d.", moveval[r][c]);
      }

      searchcontinue = movecont[r][c];
      *trow = r;  *tcol = c;  havetarget = moveval[r][c];
    }

    type = SAFE;

    while (1) {
      for (k=0; k<8; k++) {
        register int S;

        /* examine adjacent squares. */
        nr = r + sdeltr[k];
        nc = c + sdeltc[k];
        S = scrmap[nr][nc];

        /* IF we have not considered stepping on the square yet */
        /* and if it is accessible    THEN: Put it on the queue */
        if (dir[nr][nc] == NOTTRIED && (CANGO&S) && (type&S) == type &&
            (k<4 || onrc (CANGO,r,nc) && onrc (CANGO,nr,c))) {
          moveval[nr][nc] = NONE;  /* flag unevaluated */

          *(tail++) = nr;  *(tail++) = nc; if (tail == end) tail = begin;

          dir[nr][nc] = sdirect[k];  /* direction we used to get here */

          if (debug (D_SCREEN | D_SEARCH | D_INFORM))
            { at (nr, nc); printw ("%c", ">/^\\</v\\  ~"[dir[nr][nc]]);}
        }
      }

      if (type == 0) break;

      type = 0;
    }
  }
}
