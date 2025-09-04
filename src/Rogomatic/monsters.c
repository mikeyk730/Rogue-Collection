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
 * monsters.c:
 *
 * This file contains all of the monster specific functions.
 */

# include <stdio.h>
# include <ctype.h>
# include <curses.h>
# include <string.h>
# include "types.h"
# include "globals.h"

# define ADJACENT(m) (max (abs (mlist[m].mrow - atrow),\
			   abs (mlist[m].mcol - atcol)) == 1)

/*
 * monname: Return a monster name given letter '@ABC..Z'
 */

char *monname (m)
char m;
{
  return (monhist[monindex[m-'A'+1]].m_name);
}

/*
 * addmonster: add a monster to this level. Remove any monsters on the
 * list which are in the same square.
 */

void addmonster (ch, r, c, quiescence)
char  ch;
int   r, c, quiescence;
{
  char *monster = monname (ch);

  if (r > 1 || c > 3) {
    if (isholder (monster)) quiescence = AWAKE;

    deletemonster (r, c);
    mlist[mlistlen].chr = ch;
    mlist[mlistlen].mrow = r;
    mlist[mlistlen].mcol = c;
    mlist[mlistlen].q = quiescence;

    if (++mlistlen >= MAXMONST) dwait (D_FATAL, "Too many monsters");

    setrc (MONSTER, r, c);
    lyinginwait = 0;
    new_arch = 1;

    /* If we can see it, it is not really invisible */
    if (stlmatch (monster, "invisible") || streq (monster, "phantom"))
      beingstalked = 0;
  }
}

/*
 * deletemonster: remove a monster from the list at location (row, col).
 */

void deletemonster (r, c)
int   r, c;
{
  int   i;

  new_arch = 1;
  unsetrc (MONSTER, r, c);

  for (i = 0; i < mlistlen; ++i)
    if (mlist[i].mcol == c && mlist[i].mrow == r)
      { mlist[i] = mlist[--mlistlen]; i--; }
}

/*
 * dumpmonsters: (debugging) dump the list of monsters on this level.
 */

void dumpmonster ()
{
  int   i;
  at (1, 0);

  for (i = 0; i < mlistlen; ++i)
    printw ("%s at %d,%d(%c) \n",
            mlist[i].q == AWAKE ? "alert" :
            mlist[i].q == ASLEEP ? "sleeping" :
            mlist[i].q == HELD ? "held" : "unknown",
            mlist[i].mrow, mlist[i].mcol,
            mlist[i].chr);

  printw ("You are at %d,%d.", atrow, atcol);
  at (row, col);
}

/*
 * sleepmonster: Turn all unknown monsters into sleeping monsters.
 * This routine is called after we have executed a command, so if
 * the value of ASLEEP is not overridden by the monsters movement,
 * it sat still for a turn and must be asleep.
 */

void sleepmonster ()
{
  register int m;

  for (m = 0; m < mlistlen; ++m) {
    if (mlist[m].q == 0 && ! ADJACENT (m)) { //todo: why not adjacent?
      dwait (D_MONSTER, "Found a sleeping %s at %d,%d",
             monname (mlist[m].chr), mlist[m].mrow, mlist[m].mcol);

      mlist[m].q = ASLEEP;
    }
  }
}

void hold_monster(int m)
{
    dwait(D_MONSTER, "Holding %s at %d,%d",
        monname(mlist[m].chr),
        mlist[m].mrow,
        mlist[m].mcol);

    mlist[m].q = HELD;
}

int is_held(int m)
{
    return mlist[m].q == HELD;
}


/*
 * holdmonsters: Mark all close monsters as being held.
 */

void holdmonsters ()
{
  register int m;

  for (m = 0; m < mlistlen; ++m) {
    if (mlist[m].q == 0 &&
        (max (abs (mlist[m].mrow - atrow),
              abs (mlist[m].mcol - atcol)) < 3)) {
        hold_monster(m);
    }
  }
}

/*
 * wakemonster: Turn monsters into waking monsters
 *
 * dir = 0-7 means wake up adjacent plus monster in that dir
 * dir = 8   means wake up only adjacent monster
 * dir = ALL means wake up all monsters
 * dir = -m  means wake up all adjacent monsters of type m.
 */

void wakemonster (int dir)
{
  register int m;

  for (m = 0; m < mlistlen; ++m) {
    if (mlist[m].q != AWAKE &&
        (dir == ALL ||
         (dir < 0 && ADJACENT(m) && mlist[m].chr == -dir + 'A' - 1) ||
         (dir >= 0 && dir < 8 &&
          mlist[m].mrow == atdrow(dir) && mlist[m].mcol == atdcol(dir)))) {
      dwait (D_MONSTER, "Waking up %s at %d,%d",
             monname (mlist[m].chr), mlist[m].mrow, mlist[m].mcol);

      int skip_wake = mlist[m].q == HELD && dir < 0;
      if (!skip_wake)
      {
          mlist[m].q = AWAKE;
          setrc(EVERCLR, mlist[m].mrow, mlist[m].mcol);
      }
      else
      {
          dwait(D_ERROR, "Not waking up HELD %s at %d,%d after damage",
              monname(mlist[m].chr), mlist[m].mrow, mlist[m].mcol);
      }
    }
  }
}

/*
 * seemonster: Return true if a particular monster is on the monster list.
 */

int seemonster (monster)
char *monster;
{
  register int m;

  for (m = 0; m < mlistlen; ++m)
    if (streq (monname (mlist[m].chr), monster))
      return (1);

  return (0);
}

/*
 * seeawakemonster: Returns true if there is a particular awake
 * monster on the monster list.		DR UTexas 26 Jan 84
 */

seeawakemonster (monster)
char *monster;
{
  register int m;

  for (m = 0; m < mlistlen; ++m)
    if (streq (monname (mlist[m].chr), monster) && mlist[m].q == AWAKE)
      return (1);

  return (0);
}

/*
 * monsternum: Given a string e.g. "umber hulk", return the monster
 *             number from 0 to 26, e.g. "umber hulk" ==> 21. Zero
 *             is used for unknown monsters (e.g. "it").
 */

int monsternum (monster)
char *monster;
{
  int m, mh;

  if ((mh = findmonster (monster)) != NONE)
    for (m=0; m<=26; m++)
      if (monindex[m] == mh) return (m);

  return (0);
}

/*
 * newmonsterlevel: Starting a new level. Set the initial sleep status of
 * each monster.
 */

void newmonsterlevel ()
{
  register int m;
  register char *monster;

  for (m=0; m<mlistlen; m++) {
    monster = monname (mlist[m].chr);

    if (streq (monster, "floating eye")   ||
        streq (monster, "leprechaun")     ||
        streq (monster, "nymph")          ||
        streq (monster, "ice monster"))
      mlist[m].q = ASLEEP;
    else
      mlist[m].q = 0;
  }
}

/*
 * isholder: Return true if the monster can hold us.
 */

int isholder (monster)
register char *monster;
{
  return (streq (monster, "venus flytrap") || streq (monster, "violet fungi"));
}
