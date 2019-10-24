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
 * explore.c:
 *
 * This file contains all of the functions which are used to search out
 * paths to explore, pick up something, or run away.
 */

# include <curses.h>
# include "types.h"
# include "globals.h"

# define SEARCHES(r,c)						\
	(onrc(DEADEND,r,c) ?					\
	    ((version < RV53A || !isexplored (r,c)) ?		\
		(timestosearch + k_door / 5) :			\
		(timestosearch - k_door / 5 + 5)) :		\
	    timestosearch)

static int expDor, expavoidval;
static int avdmonsters[24][80];

int rogo_connect[9][4] = {
  /* Room  top    bot   left  right*/
  /* 0 */  {-1,     3,    -1,     1},
  /* 1 */  {-1,     4,     0,     2},
  /* 2 */  {-1,     5,     1,    -1},
  /* 3 */  { 0,     6,    -1,     4},
  /* 4 */  { 1,     7,     3,     5},
  /* 5 */  { 2,     8,     4,    -1},
  /* 6 */  { 3,    -1,    -1,     7},
  /* 7 */  { 4,    -1,     6,     8},
  /* 8 */  { 5,    -1,     7,    -1}
};

/*
 * genericinit: Initialize a 'standard' movement search.	MLM
 */

genericinit ()
{
  expavoidval = avoid();
  return (1);
}

/* Secret door search values and continuance tables:
 *
 *  Number of unsearched walls adjacent.
 *
 * 	Vert	Horiz	Value		Cont	Explanation
 *	---------------------------------------------------
 *	0	0	0		0	Valueless
 *	0	1	N-24-dep	16	Prefer (0,2) 1 move later
 *	0	2	N-22-dep	15	Prefer (0,3) 1 move later
 *	0	3	N-20-dep	14	Prefer (1,0) 10 moves later
 *	1	0	N-9-dep		4	Prefer (1,1) 2 moves later
 *	1	1	N-6-dep		2	Prefer (2,0) 1 move later
 *	1	2	N-5-dep		2	Prefer (2,0) 1 move later
 *	1	3	N-5-dep		2	Impossible
 *	2	0	N-3-dep		1	Prefer (3,0) 1 move later
 *	2	1	N-2-dep		0
 *	2	2	N-1-dep		0
 *	2	3	N-1-dep		0	Impossible
 *	3	0	N		0	Best possible
 *	3	1	N		0	Impossible
 *	3	2	N		0	Impossible
 *	3	3	N		0	Impossible
 */

# define N 100
static int secretvalues[16]= { 0, N-24, N-22, N-20,
                               N-9,  N-6,  N-5,  N-5,
                               N-3,  N-2,  N-1,  N-1,
                               N,    N,    N,    N
                             };

static int secretcont[16] =  { 0, 16, 15, 14,
                               4,  2,  2,  2,
                               1,  0,  0,  0,
                               0,  0,  0,  0
                             };

/*
 * gotowards: Move toward a square.
 */

int gotorow = NONE, gotocol = NONE;

gotowards (r, c, running)
int r, c, running;
{
  int gotoinit(), gotovalue();

  gotorow = r; gotocol = c;
  return (makemove (running ? RUNAWAY:GOTOMOVE, gotoinit, gotovalue, REUSE));
}

/*
 * gotoinit: Initialize a gotowards move.
 */

gotoinit ()
{
  expavoidval = avoid();
  return (1);
}

/*
 * gotovalue: Only the current target square has a value.
 */

/* ARGSUSED */
gotovalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  *avd = onrc (SAFE, r, c)    ? 0 :
         onrc (ARROW, r, c)   ? 50 :
         onrc (TRAPDOR, r, c) ? 175 :
         onrc (TELTRAP, r, c) ? 50 :
         onrc (GASTRAP, r, c) ? 50 :
         onrc (BEARTRP, r, c) ? 50 :
         onrc (DARTRAP, r, c) ? 200 :
         onrc (WATERAP, r, c) ? 50 :
         onrc (MONSTER, r, c) ? 150 :
         expavoidval;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    *avd += 200;

  *val = r == gotorow && c == gotocol ? 1 : 0;
  /*  *cont = 0; // default value when called // */
  return (1);
}

/*
 * sleepvalue: Squares with sleeping monsters have value.
 *             Use genericinit.		MLM
 */

/* ARGSUSED */
sleepvalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  *avd = onrc (SAFE, r, c)    ? 0 :
         onrc (ARROW, r, c)   ? 50 :
         onrc (TRAPDOR, r, c) ? 175 :
         onrc (TELTRAP, r, c) ? 50 :
         onrc (GASTRAP, r, c) ? 50 :
         onrc (BEARTRP, r, c) ? 50 :
         onrc (DARTRAP, r, c) ? 200 :
         onrc (WATERAP, r, c) ? 50 :
         onrc (MONSTER, r, c) ? 150 :
         expavoidval;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    *avd += 200;

  if (onrc (SLEEPER, r, c)) {
    *val = 1;
    *avd = 0;
    /* *cont = 0;  // default value when called // */
  }

  return (1);
}

/*
 * wallkind: given a row and column, determine which kind of wall if any
 *           is there. Part of doorinit   Guy Jacobson 5/82
 */

int wallkind (r, c)
int r, c;
{
  switch (screen[r][c]) {

    case '|': if (onrc (ROOM, r, c+1)) return (LEFTW);
      else return (RIGHTW);

    case '-': if (onrc (ROOM, r+1, c)) return (TOPW);
      else if (onrc (ROOM, r-1, c)) return (BOTW);
      else return (CORNERW);

    case '+': return (DOORW);
    default:  return (NOTW);
  }
}

/*
 * setpsd: Initialize secret door search
 *
 * Guy Jacobson 5/82
 * Modified to allow searching even when there are cango bits.	MLM.   10/82
 * Modified to reuse existing map while it is valid.		LGCH.  10/82
 * Modified to understand maze room secret doors.		MLM.   10/83
 */

setpsd (print)
{
  register int i, j, k, whereto, numberpsd=0;

  if (!print && reusepsd > 0) return (reusepsd-1);

  /* find what rooms are missing */
  markmissingrooms ();

  /* Changed loop boundaries to ignore border around screen -- mlm 5/18/82 */
  for (i=2; i<22; i++) for (j=1; j<79; j++) {
      unsetrc (PSD|DEADEND,i,j);

      /* If attempt > 3, allow ANYTHING to be a secret door! */
      if (attempt > 3 && ! onrc (BEEN|DOOR|HALL|ROOM|WALL|STAIRS, i, j) &&
          nextto (CANGO, i, j))
        { if (!onrc (PSD, i, j)) numberpsd++; setrc(PSD,i,j); }

      /* Set Possible Secret Door for maze room secret doors */
      else if (attempt > 0 && ! onrc (BEEN|DOOR|HALL|ROOM|WALL|STAIRS, i, j) &&
               mazedoor (i, j))
        { if (!onrc (PSD, i, j)) numberpsd++; setrc(PSD,i,j); }

      /* Set Possible Secret Door for corridor secret door */
      else if (version >= RV53A &&
               ! onrc (BEEN|DOOR|HALL|ROOM|WALL|STAIRS, i, j) &&
               nextto (DOOR, i, j))
        { if (!onrc (PSD, i, j)) numberpsd++; setrc(PSD,i,j); }

      /* Set Possible Secret Door for dead end corridors */
      else if (! onrc (BEEN|DOOR|HALL|ROOM|WALL|STAIRS, i, j) &&
               (onrc (HALL, i-1, j) + onrc (HALL, i+1, j) +
                onrc (HALL, i, j-1) + onrc (HALL, i, j+1) == HALL) &&
               !(onrc(HALL|DOOR, i-1, j-1) || onrc(HALL|DOOR, i+1, j+1) ||
                 onrc(HALL|DOOR, i-1, j+1) || onrc(HALL | DOOR, i+1, j-1)) &&
               canbedoor (i, j)) {
        if (!onrc (PSD, i, j)) numberpsd++;

        setrc(DEADEND,i,j); setrc(PSD,i,j);
      }

      /*
       * Set PSD for walls which connect to empty space
       *
       * Modified to allow PSD for !ROOM (including the old CANGO des.)
       * since potions and scrolls of detection can cause the CANGO bit to
       * be set for a square on which we have never been.
       * mlm 10/8/82
       *
       * If attempt > 2, then relax the constraint about empty space,
       * since we might have teleported into a disconnected part of the
       * level. This means after we have searched twice, we look for ANY
       * possible door, not just doors leading to empty space.
       * mlm 10/11/82
       *
       * Check return code from whichroom.
       * mlm 03/17/85
       */

      else {
        if ((k = wallkind (i,j)) >= 0) {	/* A legit sort of wall */
          register int rm = whichroom (i,j);

          if (rm >= 0 && rm < 9) {
            whereto = rogo_connect[rm][k];

            if (whereto >= 0 && whereto < 9 &&
                (attempt > 1 || room[whereto] == 0)) {
              if (!onrc (PSD, i, j)) numberpsd++;

              setrc (PSD,i,j);
            }
          }
        }
      }
    }

  /* Now remove PSD bits from walls which already have doors */
  for (i=2; i<22; i++) for (j=1; j<79; j++) {
      if (onrc (DOOR, i, j)) {
        for (k = i-1; onrc (WALL, k, j); k--)
          { if (onrc (PSD, k, j)) numberpsd--; unsetrc (PSD, k, j);}

        for (k = i+1; onrc (WALL, k, j); k++)
          { if (onrc (PSD, k, j)) numberpsd--; unsetrc (PSD, k, j);}

        for (k = j-1; onrc (WALL, i, k); k--)
          { if (onrc (PSD, i, k)) numberpsd--; unsetrc (PSD, i, k);}

        for (k = j+1; onrc (WALL, i, k); k++)
          { if (onrc (PSD, i, k)) numberpsd--; unsetrc (PSD, i, k);}
      }
    }

  if (print || debug (D_SCREEN))
    for (i=0; i<24; i++) for (j=0; j<80; j++)
        if (onrc (PSD,i,j)) { at (i,j); addch ('P'); }

  reusepsd = numberpsd+1;
  return (numberpsd);
}

/*
 * downvalue: find nearest stairs or trapdoor (use genericinit for init).
 */

/* ARGSUSED */
downvalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  *avd = onrc (SAFE, r, c)    ? 0 :
         onrc (ARROW, r, c)   ? 50 :
         onrc (TRAPDOR, r, c) ? 175 :
         onrc (TELTRAP, r, c) ? 50 :
         onrc (GASTRAP, r, c) ? 50 :
         onrc (BEARTRP, r, c) ? 50 :
         onrc (DARTRAP, r, c) ? 200 :
         onrc (WATERAP, r, c) ? 50 :
         onrc (MONSTER, r, c) ? 150 :
         expavoidval;

  if (onrc (STAIRS | TRAPDOR, r, c)) { *val = 1; *avd = 0; }
  else			             { *val = 0; }

  return (1);
}

/*
 * expruninit: same as expinit but don't bias against doors.
 */

expruninit ()
{
  dwait (D_CONTROL | D_SEARCH, "expruninit called.");
  expinit();
  expDor = 0;
  avoidmonsters ();
  return (1);
}

/*
 * exprunvalue: When running, avoid monsters.
 *
 * Try to see a new square when running.
 */

exprunvalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  if (r == atrow && c == atcol)		/* Current square useless MLM */
    *val = 0;
  else if (onrc (MONSTER | TRAP, r, c))	/* Added TRAP useless MLM */
    *val = 0;
  else if (!zigzagvalue (r, c, depth, val, avd, cont))
    return (0);

  if (*val > 0) { *val = *val * 1000 + depth; *cont = ROGINFINITY; }

  *avd += avdmonsters[r][c];
  return (1);
}

/*
 * expunpininit: same as exprunnit but try to unpin.
 */

expunpininit ()
{
  dwait (D_CONTROL | D_SEARCH, "expunpininit called.");
  expinit();
  expDor = 0;
  pinavoid ();
  return (1);
}

/*
 * expunpinvalue: When unpinning, avoid monsters.
 *
 * Try to see a new square when unpinning, but unpin anywhere if need be.
 */

expunpinvalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  if (r == atrow && c == atcol)		/* Current square useless MLM */
    *val = 0;
  else if (onrc (MONSTER | TRAP, r, c))	/* Added TRAP useless MLM */
    *val = 0;
  else if (!zigzagvalue (r, c, depth, val, avd, cont))
    return (0);

  if (*val > 0) { *val = *val * 1000 + depth; *cont = ROGINFINITY; }

  *avd += avdmonsters[r][c];
  return (1);
}

/*
 * runinit:  R U N   A W A Y   S E A R C H
 */

runinit ()
{
  avoidmonsters();
  return (1);
}

/*
 * runvalue:
 *
 * Evaluate square for running away. Targets, in priority order are:
 *
 * STAIRS  TRAPDOR  TELTRAP
 * RUNOK (cycle door)
 * DOOR
 * ANYWHERE
 *
 * Traps are avoided for a variable number of moves, except for target traps
 * Gave GasTraps and BearTraps infinite avoidance.	MLM 10/11/83
 */

runvalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  *avd = onrc (ARROW, r, c) ? 50 :
         onrc (TRAPDOR, r, c) ? 0 :
         onrc (TELTRAP, r, c) ? 0 :
         onrc (GASTRAP, r, c) ? ROGINFINITY :
         onrc (BEARTRP, r, c) ? ROGINFINITY :
         onrc (DARTRAP, r, c) ? 100 :
         onrc (WATERAP, r, c) ? 100 :
         onrc (MONSTER, r, c) ? 150 :
         0;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    *avd += 200;

  if (onrc (MONSTER, r, c))
    { *val = 0; }

  if (onrc (STAIRS+TRAPDOR+TELTRAP, r, c))
    { *val = 5000; *avd = 0; /* *cont = 0; */ }
  else if (r == atrow && c == atcol)	/* If we are running, our current */
    { *val = 0;}			/* cant be that great -- MLM      */
  else if (onrc (RUNOK, r, c))
    { *val = 4000; *cont = ROGINFINITY;}
  else if (onrc (DOOR | BEEN, r, c) == DOOR)
    { *val = 2000+depth; *cont = ROGINFINITY;}
  else if (onrc (DOOR, r, c))
    { *val = 1000+depth; *cont = ROGINFINITY;}
  else if (onrc (HALL, r, c))
    { *val =      depth; *cont = ROGINFINITY;}
  /* ----------------------------------------------------------------
  else if (onrc (CANGO | TRAP, r, c) == CANGO)
    { *val = 1+depth; *cont = ROGINFINITY;}
  ---------------------------------------------------------------- */
  else
    { *val = 0; }

  *avd += avdmonsters[r][c];
  return (1); /* mdk:bugfix eliminated undefined behavior */
}

/*
 * unpininit:  U N P I N    S E A R C H
 *
 * Same as runint, but we are willing to take one hit to get away.
 */

unpininit ()
{
  pinavoid();
  return (1);
}

/*
 * rundoorinit: Standard initialization routine.
 */

rundoorinit()
{
  avoidmonsters();
  return (1);
}

/*
 * rundoorvalue:
 *
 * Evaluate square for running into doorway.
 *
 * Targets, in priority order are:
 *
 * RUNOK (cycle door)
 * DOOR
 *
 * Traps are avoided for a variable number of moves, except for target traps
 * Gave GasTraps and BearTraps infinite avoidance.	MLM 10/11/83
 */

/* ARGSUSED */
rundoorvalue (r, c, depth, val, avd, cont)
int r, c, depth;
int *val, *avd, *cont;
{
  *avd = onrc (ARROW, r, c) ? 50 :
         onrc (TRAPDOR, r, c) ? 0 :
         onrc (TELTRAP, r, c) ? 0 :
         onrc (GASTRAP, r, c) ? ROGINFINITY :
         onrc (BEARTRP, r, c) ? ROGINFINITY :
         onrc (DARTRAP, r, c) ? 100 :
         onrc (WATERAP, r, c) ? 100 :
         onrc (MONSTER, r, c) ? 50 : 0;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    *avd += 200;

  if (onrc (RUNOK, r, c))	{ *val = 2;}
  else if (onrc (DOOR, r, c))	{ *val = 1; *cont = ROGINFINITY;}
  else				{ *val = 0;}

  *avd += avdmonsters[r][c];
  return (1); /* mdk:bugfix eliminated undefined behavior */
}

/*
 *   E X P L O R A T I O N   S E A R C H
 */

expinit ()
{
  /* avoidance values for doors */
  expDor = 0;
  expavoidval = avoid();
  return (1);
}

roominit ()
{
  expinit ();
  expDor = ROGINFINITY;
  return (1);
}

/*
 * expvalue: evaluation function for exploration.  LGCH
 *
 * In order to prevent leaving orphan unseen squares, we have heuritics
 * which cause rogomatic to use the three-step pattern to scan along the
 * boundary of a room, and also have tests which detect any orphans (e.g.
 * corners and when an object interrupts the search pattern) and cause them
 * to be seen.
 *
 * Three-step pattern:
 *
 *        @@ @@
 *       @  @  @
 *      bbbbbbbbb
 */

/* ARGSUSED */
expvalue (r, c, depth, val, avd, cont)
int r, c, depth;
int *val, *avd, *cont;
{
  register int k, nr, nc, l;
  int a, v = 0, nunseenb = 0, nseenb = 0, nearb = 0;

  a = onrc (SAFE|DOOR|STAIRS|HALL, r, c) ? 0 :
      onrc (ARROW, r, c)   ? 50 :
      onrc (TRAPDOR, r, c) ? 300 :
      onrc (TELTRAP, r, c) ? 100 :
      onrc (GASTRAP, r, c) ? 50 :
      onrc (BEARTRP, r, c) ? 50 :
      onrc (DARTRAP, r, c) ? 200 :
      onrc (TRAP, r, c)    ? 100 :
      onrc (MONSTER, r, c) ? 150 :
      expavoidval;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    { *avd = a+1000; *val=0; return (1); }

  if (onrc (BEEN+SEEN, r, c) == SEEN) { /* If been or not seen, not a target */
    for (k=0; k<8; k++) {
      nr = r + deltr[k];
      nc = c + deltc[k];

      /* For each unseen neighbour: add 10 to value. */
      if (nr >= 1 && nr <= 22 && nc >= 0 && nc <= 80 &&
          !onrc (SEEN, nr, nc)) {
        v += 10;

        if (onrc (BOUNDARY, nr, nc)) {
          /* Count unseen boundary neighbours. */
          nunseenb++;

          /* Count seen boundaries horiz/vert adjacent to unseen boundary */
          for (l=0; l<8; l+=2)
            if (onrc (SEEN+BOUNDARY, nr+deltr[l], nc+deltc[l]) ==
                SEEN+BOUNDARY)
              nseenb++;

        }
        else {
          /* Check for unseen boundary horiz/vert       */
          /* adjacent to neighbour and not a neighbour. */
          l = k / 2 * 2; 		     /* horizontal/vertical */

          if (onrc (BOUNDARY+SEEN, nr+deltr[l], nc+deltc[l]) == BOUNDARY)
            nearb = 1;
          else {
            l = ((k+1) / 2 * 2) % 8;

            if (onrc (BOUNDARY+SEEN, nr+deltr[l], nc+deltc[l]) == BOUNDARY)
              nearb = 1;
          }
        }
      }
    }

    /* To zig-zag: add number of unseen boundary neighbours * 6 */
    v += nunseenb * 6;

    /* To do the three-step: add 29 if an unseen neighbour had an unseen
     * boundary horiz/vert adjacent */
    if (nearb)
      v += 29;

    /* To prevent orphans: if three unseen neighbours are boundary and one
     * has a seen boundary horiz/vert adjacent, add 200 */
    if (nunseenb >= 3 && nseenb >= 1)
      v += 200;

    /* To clean up any orphans: if two seen boundaries are adjacent to any
     * unseen boundary neighbours, add 400. */
    if (nseenb >= 2)
      v += 400;
  }

  if (onrc (DOOR, r, c))
    a += expDor;

  /* else if (onrc (BEEN+BOUNDARY, r, c) == BOUNDARY)
    a++;   // Avoid running along the untrodden boundary. // */

  *avd = a;
  *val = v;

  if (v < 50)
    *cont = 4; 				     /* Look for something better */

  if (debug (D_SCREEN) && v > 0)
    { mvaddch (r, c, 'o'); dwait (D_SCREEN, "Value %d", v); }

  return (1);
}

/*
 * zigzagvalue: evaluation function for exploration.  LGCH
 *
 * This is a copy of expvalue with the three-step code and the code to
 * detect orphans removed. It is used when running away and unpinning
 * to find a useful exploration move. The boundary moves must zig-zag
 * rather than three-step so that the door can be entered when it is seen
 * without taking a hit from the monster chasing us.
 *
 * Gave GasTraps and BearTraps infinite avoidance.	MLM 10/11/83
 */

/* ARGSUSED */
zigzagvalue (r, c, depth, val, avd, cont)
int r, c, depth;
int *val, *avd, *cont;
{
  register int k, nr, nc, a, v = 0, nunseenb = 0;

  a = onrc (SAFE|DOOR|STAIRS|HALL, r, c) ? 0 :
      onrc (ARROW, r, c)   ? 50 :
      onrc (TRAPDOR, r, c) ? 300 :
      onrc (TELTRAP, r, c) ? 100 :
      onrc (GASTRAP, r, c) ? 50 :
      onrc (BEARTRP, r, c) ? 50 :
      onrc (DARTRAP, r, c) ? 200 :
      onrc (TRAP, r, c)    ? 100 :
      onrc (MONSTER, r, c) ? 150 :
      expavoidval;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    { *avd = a+1000; *val=0; return (1); }

  if (onrc (BEEN+SEEN, r, c) == SEEN) { /* If been or not seen, not a target */
    for (k=0; k<8; k++) {
      nr = r + deltr[k];
      nc = c + deltc[k];

      /* For each unseen neighbour: add 10 to value. */
      if (nr >= 1 && nr <= 22 && nc >= 0 && nc <= 80 &&
          !onrc (SEEN, nr, nc)) {
        v += 10;

        if (onrc (BOUNDARY, nr, nc)) {
          /* Count unseen boundary neighbours. */
          nunseenb++;
        }

        if (debug (D_SCREEN)) mvaddch (nr, nc, 'o');
      }
    }

    /* To zig-zag: add number of unseen boundary neighbours * 6 */
    v += nunseenb * 6;
  }

  if (onrc (DOOR, r, c))
    a += expDor;

  /* else if (onrc (BEEN+BOUNDARY, r, c) == BOUNDARY)
    a++;   // Avoid running along the untrodden boundary. // */

  *avd = a;
  *val = v;
  *cont = 0; 				     /* Look for orphans */

  return (1);
}

/*
 *   S E C R E T   D O O R   S E A R C H
 */

secretinit ()
{
  expinit ();

  if (setpsd (NOPRINT))
    return (1);

  return (0);
}

/* ARGSUSED */
secretvalue (r, c, depth, val, avd, cont)
int r, c, depth;
int *val, *avd, *cont;
{
  register int v, a, k;

  *val=0;
  v = 0;	/* establish value of square */
  a = onrc (SAFE, r, c)    ? 0 :
      onrc (ARROW, r, c)   ? 50 :
      onrc (TRAPDOR, r, c) ? 175 :
      onrc (TELTRAP, r, c) ? 50 :
      onrc (GASTRAP, r, c) ? 50 :
      onrc (BEARTRP, r, c) ? 50 :
      onrc (DARTRAP, r, c) ? 200 :
      onrc (WATERAP, r, c) ? 50 :
      onrc (MONSTER, r, c) ? 150 :
      expavoidval;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    a += 200;

  for (k=0; k<8; k++) {  /* examine adjacent squares */
    register int nr = r + deltr[k];
    register int nc = c + deltc[k];

    if (nr >= 1 && nr <= 22 &&
        nc >= 0 && nc <= 80 &&
        onrc (PSD, nr, nc) && timessearched[nr][nc] < SEARCHES(nr,nc)) {
      /* If adjacent square is on the screen */
      /* and if it has PSD set but has not been searched completely */
      /* count useful neighbours */
      if (screen[nr][nc] == '|')	v += 4;
      else				v ++;

      if (debug (D_SCREEN | D_INFORM)) mvaddch (nr, nc, 'S');
    }
  }

  if (v>0) {
    if (version >= RV53A &&
        onrc (DOOR|BEEN, r, c) == (DOOR|BEEN) &&
        (onrc (CANGO|WALL, r+1, c) == 0 || onrc (CANGO|WALL, r-1, c) == 0 ||
         onrc (CANGO|WALL, r, c+1) == 0 || onrc (CANGO|WALL, r, c-1) == 0))
      { *val = v+100; *cont = 5; }
    else {
      v = min (15, v);
      *val = secretvalues[v]; *cont = secretcont[v];

      if (onrc (DOOR, r, c)) a += expDor;
    }
  }

  *avd = a;

  return (1);
}

/*
 *   E S T A B L I S H   A V O I D A N C E   M A P   to avoid monsters.
 */

# define AVOID(r,c,ch) \
  { avdmonsters[r][c] = ROGINFINITY; \
    if (debug (D_SCREEN)) { at((r),(c)); addch(ch); at(row,col); }}

avoidmonsters ()
{
  register int i, r, c, wearingstealth;

  /* Clear old avoid monster values */
  for (i = 24*80; i--; ) avdmonsters[0][i] = 0;

  /* Set stealth status */
  wearingstealth = (wearing ("stealth") != NONE);

  /* Avoid each monster in turn */
  for (i=0; i<mlistlen; i++) {

    /* First check whether this monster is really wimpy */
    if (maxhit(i) < Hp/2) {
      AVOID (mlist[i].mrow, mlist[i].mcol, '$')
    }

    /* If not a wimp and awake, avoid him all together */
    else if (mlist[i].q == AWAKE) {
      int d, dr, dc, mr = mlist[i].mrow, mc = mlist[i].mcol;
      d = direc (searchstartr-mr,searchstartc-mc);
      dr = (searchstartr-mr)/2+mr; dc=(searchstartc-mc)/2+mc;

      if (d & 1) {
        caddycorner (dr, dc, (d-1) & 7, (d-3)&7, '$');
        caddycorner (dr, dc, (d+1) & 7, (d+3)&7, '$');
      }
      else {
        caddycorner (dr, dc, (d-2) & 7, d, '$');
        caddycorner (dr, dc, (d+2) & 7, d, '$');
      }
    }

    /* If he'll wake up, give him a wide berth */
    else if (!wearingstealth) {
      for (r = mlist[i].mrow-1; r<= mlist[i].mrow+1; r++)
        for (c = mlist[i].mcol-1; c<= mlist[i].mcol+1; c++)
          if (!onrc (WALL, r, c))
            AVOID (r, c, '$')
    }

    /* He's asleep, don't try to run through him */
    else
      AVOID (mlist[i].mrow, mlist[i].mcol, '$')
  }

  /* Don't avoid current position */
  avdmonsters[searchstartr][searchstartc] = 0;
  dwait (D_SEARCH, "Avoidmonsters: avoiding the $s");
}

/*
 * caddycorner: Find sqaures the monster can reach before we can and mark
 * them for avoidance
 */

caddycorner (r, c, d1, d2, ch)
int r,c,d1,d2;
char ch;
{
  while (onrc (CANGO, r, c)) {
    AVOID (r, c, ch);
    r += deltr[d1]; c += deltc[d1];

    if (!onrc (CANGO, r, c)) break;

    AVOID (r, c, ch);
    r += deltr[d2]; c += deltc[d2];
  }
}

/*
 *   E S T A B L I S H   A V O I D A N C E   M A P   when pinned
 *
 * This routine is basically the same as avoidmonsters, except that
 * the value are calculated to allow the monster one hit instead of
 * avoiding him entirely. This allows us to escape from situations where
 * we are pinned, but could get free if we had an extra turn.   MLM
 */

pinavoid ()
{
  register int i;

  /* Clear old avoid monster values */
  for (i = 24*80; i--; ) avdmonsters[0][i] = 0;

  /* Avoid each monster in turn */
  for (i=0; i<mlistlen; i++) {
    if (mlist[i].q == AWAKE) {
      register int d, dr, dc, mr = mlist[i].mrow, mc = mlist[i].mcol;
      d = direc (searchstartr-mr,searchstartc-mc);
      dr = (searchstartr-mr)/2+mr - deltr[d];		/* MLM */
      dc=(searchstartc-mc)/2+mc - deltc[d];		/* MLM */

      if (d & 1) {
        caddycorner (dr, dc, (d-1) & 7, (d-3)&7, '&');
        caddycorner (dr, dc, (d+1) & 7, (d+3)&7, '&');
      }
      else {
        caddycorner (dr, dc, (d-2) & 7, (d-4) & 7, '&'); /* MLM */
        caddycorner (dr, dc, (d+2) & 7, (d+4) & 7, '&'); /* MLM */
      }
    }

    AVOID (mlist[i].mrow, mlist[i].mcol, '&');
  }

  /* Don't avoid current position */
  avdmonsters[searchstartr][searchstartc] = 0;
  dwait (D_SEARCH, "Pinavoid: avoiding the &s");
}

/*
 *   S E C R E T : Search dead ends for secret doors.
 */

secret ()
{
  int secretinit(), secretvalue();

  /* Secret passage adjacent to door? */
  if (version >= RV53A && on (DOOR) && !blinded &&
      (seerc (' ',atrow+1,atcol) || seerc (' ',atrow-1,atcol) ||
       seerc (' ',atrow,atcol+1) || seerc (' ',atrow,atcol-1)) &&
      SEARCHES (atrow, atcol) < timestosearch+20) {
    int count = timessearched[atrow][atcol]+1;
    saynow ("Searching dead end door (%d,%d) for the %d%s time...",
            atrow, atcol, count, ordinal (count));
    command (T_DOORSRCH, "s"); return (1);
  }

  /* Verify that we are actually at a dead end */
  if (onrc (CANGO,atrow-1,atcol) + onrc (CANGO,atrow,atcol-1) +
      onrc (CANGO,atrow+1,atcol) + onrc (CANGO,atrow,atcol+1) != CANGO)
    return (0);

  /* If Level 1 or edge of screen: dead end cannot be room, mark and return */
  if (Level == 1 && attempt == 0 ||
      version < RV53A && (atrow<=1 || atrow>=22 || atcol<=0 || atcol>=79))
    { markexplored (atrow, atcol); return (0); }

  /* Have we mapped this level? */
  if (Level == didreadmap) return (0);

  /* Found a dead end, should we search it? */
  if (nexttowall (atrow, atcol) ||
      canbedoor (atrow, atcol) &&
      (version >= RV53A || !isexplored (atrow, atcol))) {
    setrc (DEADEND, atrow, atcol);

    if ((SEARCHES (atrow, atcol) - timessearched[atrow][atcol]) > 0) {
      int count = timessearched[atrow][atcol]+1;
      saynow ("Searching dead end (%d,%d) for the %d%s time...",
              atrow, atcol, count, ordinal (count));
      command (T_DOORSRCH, "s");
      return (1);
    }
    else {
      markexplored (atrow, atcol);
      return (0);
    }
  }

  return (0);
}

/*
 *   F I N D R O O M :  Try to find another room.
 */

findroom ()
{
  int expinit(), expvalue();    /* LGCH */

  if (new_findroom) {
    if (!on (ROOM) && secret ())			return (1);

    if (makemove (EXPLORE, expinit, expvalue, REUSE))	 return (1);
  }

  new_findroom = 0;
  dwait (D_SEARCH, "findroom failed.");
  return (0);
}

/*
 *   E X P L O R E   R O O M :  Explore the current room.
 */

exploreroom ()
{
  int roominit(), expvalue();    /* LGCH */

  if (!on (ROOM) || isexplored (atrow, atcol)) return (0);

  if (makemove (EXPLOREROOM, roominit, expvalue, REUSE)) return (1);

  markexplored (atrow, atcol);

  dwait (D_SEARCH, "exploreroom failed.");
  return (0);
}

/*
 *   D O O R E X P L O R E : look for secret doors
 */

doorexplore()
{
  static searchcount = 0;
  int secretinit(), secretvalue();

  /* If no new squares or read map, dont bother */
  if (! new_search || Level == didreadmap)
    { searchcount = 0; return (0); }

  if (makemove (SECRETDOOR, secretinit, secretvalue, REUSE))  /* move */
    { searchcount = 0; return (1); }

  if (searchcount > 20)
    { new_search = 0; return (0); }

  if (ontarget) { /* Moved to a possible secret door, search it */
    searchcount++;
    saynow ("Searching square (%d,%d) for the %d%s time...",
            atrow, atcol, searchcount, ordinal (searchcount));
    command (T_DOORSRCH, "s");
    return (1);
  }

  new_search = searchcount = 0;
  return (0);
}

/*
 *   S A F E   S Q U A R E   S E A R C H 	Use genericinit.
 */

safevalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  register int k, v;

  *avd = onrc (SAFE, r, c)    ? 0 :
         onrc (TRAPDOR | BEARTRP | GASTRAP, r, c) ? ROGINFINITY :
         onrc (ARROW, r, c)   ? 50 :
         onrc (TELTRAP, r, c) ? 50 :
         onrc (DARTRAP, r, c) ? 200 :
         onrc (WATERAP, r, c) ? 50 :
         onrc (MONSTER, r, c) ? 150 :
         expavoidval;
  *val = 0;

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    *avd += 500;

  if (onrc(CANGO, r, c)) {
    v = 0;

    for (k=0; k<8; k++)
      if (onrc(CANGO, r+deltr[k], c+deltc[k]) &&
          onrc(CANGO, r+deltr[k], c) &&
          onrc(CANGO, r, c+deltc[k])) v++;

    if (v < 3)
      *val = 1;        /* *cont = 0; // default // */
  }

  return (1);
}

/* findsafe: find a spot with 2 or fewer moves, for when blinded */

findsafe()
{
  return (makemove (FINDSAFE, genericinit, safevalue, REEVAL));
}

/* How scared are we of hitting a trap? */

avoid ()
{
  if (cheat && !foundarrowtrap && !usingarrow) return (0);
  else if (Level < 5) return (0);       /* Don't bother */
  else return (2);                      /* Avoid a little */
}

/*
 * archery: Initialize a archery move. We find a good place to
 * shoot from and stand there. Then mark the monster as awake, and
 * battlestations will handle firing at him.
 */

static int archrow = NONE, archcol = NONE, archturns = NONE, archval[24][80];

archmonster (m, trns)
register int m;		/* Monster to attack */
register int trns;	/* Minimum number of arrows to make it worthwhile */
{
  int archeryinit(), archeryvalue();
  register int mr, mc;

  dwait (D_CONTROL | D_BATTLE, "archmonster: m=%d, turns=%d", m, trns);

  if (! new_arch) return (0);

  /* Useless without arrows */
  if (havemult (missile, "", trns) < 0) {
    dwait (D_BATTLE, "archmonster, fewer than %d missiles", trns);
    return (0);
  }

  /* For now, only work for sleeping monsters */
  if (mlist[m].q != ASLEEP)
    { dwait (D_BATTLE, "archmonster, monster not asleep"); return (0); }

  /* Save globals */
  archrow = mlist[m].mrow; archcol = mlist[m].mcol; archturns = trns;

  /* Can we get to a suitable square */
  if (makemove (ARCHERYMOVE, archeryinit, archeryvalue, REUSE))
    { dwait (D_BATTLE, "archmonster, made a move"); return (1); }

  /* If no move made and not on target, no path to monster */
  if (!ontarget) { new_arch = 0; return (0); }

  /* On target: wake him up and set darkdir/turns if necessary */
  mr = mlist[m].mrow; mc = mlist[m].mcol; targetmonster = mlist[m].chr;
  mlist[m].q = AWAKE; dwait (D_BATTLE, "archmonster, waking him up");

  /* Set dark room archery variables, add goal of standing on square */
  if (darkroom ()) {
    darkdir = direc (mr-atrow, mc-atcol);
    darkturns = max (abs (mr-atrow), abs (mc-atcol));
    agoalr = mr; agoalc = mc;	/* Go here to pick up what (s)he drops */
  }

  /* Tell the user about it */
  saynow ("Arching at %s", monname (mlist[m].chr));

  return (1);
}

/*
 * archeryinit: Initialize a archery move. Must avoid monsters to avoid
 * waking our potential victim up.
 */

archeryinit ()
{
  register int dir, r, c, dr, dc, dist;

  /* Clear the archery value array */
  for (r = 24*80; r--; ) archval[0][r] = 0;

  /* Scan around monster to see how far away we can shoot from */
  for (dir = 0; dir < 8; dir++) {
    dr = deltr[dir]; dc = deltc[dir];

    for (dist = 1, r = archrow+dr, c = archcol+dc;
         onrc (CANGO | HALL | MONSTER, r, c) == CANGO;
         r += dr, c += dc, dist++)
      if (dist > archturns && !onrc (TRAP, r, c)) {
        archval[r][c] = dist - 1; /* number of arrows we get to shoot */

        if (debug (D_SCREEN)) { at (r, c); addch ('=');  at (row, col); }
      }
  }

  expavoidval = avoid();
  avoidmonsters ();
  return (1);
}

/*
 * archeryvalue: Get the value of the square from the archery array.
 * Value is non-zero only if we can fire arrows at beast and value is
 * number of shots we can fire.
 */

/* ARGSUSED */
archeryvalue (r, c, depth, val, avd, cont)
int r, c, depth, *val, *avd, *cont;
{
  *avd = (onrc (SAFE, r, c)	? 0 :
          onrc (TRAPDOR, r, c)	? ROGINFINITY :
          onrc (HALL, r, c)	? ROGINFINITY :
          onrc (ARROW, r, c)	? 50 :
          onrc (TELTRAP, r, c)	? 50 :
          onrc (GASTRAP, r, c)	? 50 :
          onrc (BEARTRP, r, c)	? 50 :
          onrc (DARTRAP, r, c)	? 200 :
          onrc (WATERAP, r, c)	? 50 :
          onrc (MONSTER, r, c)	? 150 :
          expavoidval) + avdmonsters[r][c];

  if (onrc (SCAREM, r, c) && version < RV53A && objcount != maxobj)
    *avd += 500;

  *val = archval[r][c];
  *cont = ROGINFINITY;

  return (1);
}

/*
 *   M O V E   T O   R E S T :  Find a safe square to rest up on.
 */

static restinlight = 0;			/* True only in lit rooms */
static restinroom = 0;			/* True only in a room */
static restr = NONE, restc = NONE;	/* Square to rest on */

/* Set new resting goal */
unrest ()
{
  restr = restc = NONE;
}

/* Move to a good square to rest up on */
movetorest ()
{
  int restinit(), restvalue();    /* LGCH */

  if (markcycles (NOPRINT))
    unrest ();

  /* If we are where we want to rest, do so */
  if (restr >= 0 && atrow == restr && atcol == restc)
    { dwait (D_SEARCH, "movetorest: already on square"); return (0); }

  /* Try to move to a better square (remember position) */
  if (makemove (RESTMOVE, restinit, restvalue, REUSE)) {
    dwait (D_SEARCH, "movetorest wins.");
    restr = targetrow; restc = targetcol;
    return (1);
  }

  /* Cant move anywhere better, stay here */
  dwait (D_SEARCH, "movetorest fails.");
  restr = atrow; restc = atcol;

  return (0);
}

restinit ()
{
  expavoidval = avoid();
  restinlight = (on (ROOM) && !darkroom ());
  restinroom = on (ROOM);
  return (1);
}

/* ARGSUSED */
restvalue (r, c, depth, val, avd, cont)
register int r, c;
int depth, *val, *avd, *cont;
{
  register int dr, dc, ar, ac;
  int count, dir, rm;

  /* Find room number for diagonal selection */
  if ((rm = whichroom (r, c)) < 0) rm = 4;

  /* Default is no value, no avoidance */
  *avd = *val = 0;

  /* Set base value of square */
  if (onrc (TRAP|MONSTER,r, c))               { *avd = ROGINFINITY; return (0); }
  else if (restinroom && onrc (DOOR,r, c))    { *avd = ROGINFINITY; return (0); }
  else if (onrc (SCAREM, r, c)) {
    if (objcount == maxobj || version >= RV53A) { *val = 500; return (1); }
    else                                      { *avd = ROGINFINITY; return (0); }
  }
  else if (onrc (STAIRS, r, c))               { *val = 400; return (1); }
  else if (onrc (ROOM, r, c))                 { *val = 1; *cont = 99;}
  else if (!onrc (SAFE|BEEN|STUFF, r, c))     { *avd = 5; }

  /* Give bonus for being next to a trap door or a teleport trap */
  if (onrc (TRAPDOR|TELTRAP, r-1, c-1) || onrc (TRAPDOR|TELTRAP, r+1, c-1) ||
      onrc (TRAPDOR|TELTRAP, r-1, c+1) || onrc (TRAPDOR|TELTRAP, r+1, c+1))
    { *val += 80; *cont = 99;}

  if (onrc (TRAPDOR|TELTRAP, r-1, c) || onrc (TRAPDOR|TELTRAP, r+1, c) ||
      onrc (TRAPDOR|TELTRAP, r, c-1) || onrc (TRAPDOR|TELTRAP, r, c+1))
    { *val += 30; *cont = 99;}

  /* In lit rooms (with ammo) stay away from doors, this gives us time */
  /* to shoot arrows at monsters coming in at us          MLM 06/21/83 */
  if (restinlight && ammo) {
    for (dir = 0; dir < 8; dir += 2) {
      dr = deltr[dir]; dc = deltc[dir];

      for (count = 0, ar = r+dr, ac = c+dc;
           onrc (CANGO | HALL | MONSTER, ar, ac) == CANGO;
           ar += dr, ac += dc, count++) {
        /* Bonus of 'count' if this square covers a door */
        if (onrc(DOOR,ar+deltr[(dir+2)%8],ac+deltc[(dir+2)%8])) *val += count;

        if (onrc(DOOR,ar+deltr[(dir+6)%8],ac+deltc[(dir+6)%8])) *val += count;

        if (onrc(DOOR,ar,ac)) *val += count;
      }
    }
  }

  /* In dark rooms, stand diagonally away from doors (1 extra turn) */
  else if (onrc (ROOM, r, c)) {
    if (onrc (DOOR,r-1,c-1) && (rm!=0 && rm!=1 && rm!=3)) {*val+=80; *cont=99;}

    if (onrc (DOOR,r+1,c-1) && (rm!=3 && rm!=6 && rm!=7)) {*val+=80; *cont=99;}

    if (onrc (DOOR,r-1,c+1) && (rm!=1 && rm!=2 && rm!=5)) {*val+=80; *cont=99;}

    if (onrc (DOOR,r+1,c+1) && (rm!=5 && rm!=7 && rm!=8)) {*val+=80; *cont=99;}

    /* Bonus for door also orthogonally away */
    if(onrc(DOOR,r,c-1)||onrc(DOOR,r-1,c)||onrc(DOOR,r,c+1)||onrc(DOOR,r+1,c))
      { *val+=30; *cont=99; }
  }

  return (1);
}
