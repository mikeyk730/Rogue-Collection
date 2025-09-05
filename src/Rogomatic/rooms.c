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
 * rooms.c:
 *
 * Contains functions which deal with the geometry of
 * the dungeon levels, rooms, and passages.
 */

# include <curses.h>
# include <ctype.h>
# include <string.h>
# include "types.h"
# include "globals.h"

# define sign(x) ((x)?(x)>0?1:-1:0)
# define EXPLORED 01
# define HASROOM  02

int levelmap[9];

/*
 * newlevel: Clear old data structures and set up for a new level.
 */

void newlevel ()
{
  debuglog("level: %d\n", Level);
  if (Level == g_pause_at_level)
  {
      if (debugging == 0)
        debugging = D_NORMAL;
      dwait(D_WARNING, "Breaking at level %d", Level);
  }

  if (Level >= 20)
  {
      dwait(D_ERROR, "Good game in progress, level %d", Level);
  }

  int   i, j;

  initstufflist ();			/* Delete the list of items */
  diddrop = 0;				/* Clear dropped item flag */
  droppedscare = 0;			/* Old stuff gone */
  maxobj = 22;				/* Reset maximum # of objs */
  newmonsterlevel ();			/* Do new monster stuff */
  exploredlevel = 0;			/* New level */
  aggravated = 0;			/* Old monsters gone */
  beingstalked = 0;			/* Old monsters gone */
  darkdir = NONE; darkturns = 0;	/* Not arching old monster */
  stairrow = NONE; staircol = 0;	/* Get rid of old stairs */
  missedstairs = 0;
  newdoors = doorlist;			/* Clear door list */
  goalr = goalc = NONE;			/* Old goal invalid */
  trapr = trapc = NONE;			/* Old traps are gone */
  foundarrowtrap = foundtrapdoor = 0;   /* Old traps are gone */
  teleported = 0;			/* Not teleported yet */
  attempt = 0;				/* Haven't search for doors yet */
  usesynch = 0;				/* Force a new inventory */
  compression = Level < 13;		/* Set move compression */
  newarmor = newweapon = newring = 1;	/* Reevaluate our items */
  foundnew ();				/* Reactivate all rules */
  clearsendqueue ();	/* Clear old commands */
  searchcount = 0;
  /*
   * Clear the highlevel map
   */

  for (i = 0; i < 9; i++) levelmap[i] = 0;

  for (i = 0; i < 9; i ++) for (j = 0; j < 9; j ++) zonemap[i][j] = (i == j);

  zone = NONE;

  /*
   * Clear the lowlevel map
   */

  for (i = 1; i < STATUSROW; i++)
    for (j = 0; j < MAXCOLS; j++) {  /* Forall screen positions */
      scrmap[i][j] = SCRMINIT;
      timessearched[i][j] = 0;
      updatepos (screen[i][j], i, j);
    }

  atrow0 = atrow;
  atcol0 = atcol;
  set (ROOM);
  setnewgoal ();
  timestosearch = k_door / 5;
}

/* routine to find the rooms:
 * room[i] =     0 -- nothing in sector
 *            ROOM -- room found already
 *	     CANGO -- halls, but no room in sector
 */

static struct {int top,bot,left,right;} bounds[9]=

  /* top bot left right */
  /*0*/	{{ 1,           6,   0,           25},
  /*1*/	 { 1,           6,  27,           51},
  /*2*/	 { 1,           6,  53,  (MAXCOLS-1)},
  /*3*/	 { 8,          14,   0,           25},
  /*4*/	 { 8,          14,  27,           51},
  /*5*/	 { 8,          14,  53,  (MAXCOLS-1)},
  /*6*/	 {16, (STATUSROW-1),   0,           25},
  /*7*/	 {16, (STATUSROW-1),  27,           51},
  /*8*/	 {16, (STATUSROW-1),  53,  (MAXCOLS-1)}
};

void markmissingrooms ()
{
  register rm,i,j;

  for (rm=0; rm<9; ++rm) {
    room[rm]=0;

    for (i=bounds[rm].top; i<=bounds[rm].bot; ++i)
      for (j=bounds[rm].left; j<=bounds[rm].right; ++j)
        if (onrc(ROOM,i,j)) { room[rm]=ROOM; goto nextroom; }
        else if (onrc(BEEN,i,j)) { room[rm]=BEEN; goto nextroom; }

nextroom: ;
  }
}

/*
 * whichroom: Return the zone number of a square (0..8) or -1, as follows:
 *
 *		room 0 | room 1 | room 2
 *    -------+--------+-------
 *		room 3 | room 4 | room 5
 *    -------+--------+-------
 *		room 6 | room 7 | room 8
 */

int whichroom (r,c)
register int r,c;
{
  register int rm;

  for (rm=0; rm<9; ++rm)
    if (r >= bounds[rm].top  && r <= bounds[rm].bot &&
        c >= bounds[rm].left && c <= bounds[rm].right)
      return(rm);

  return (-1);
}

/*
 * nametrap: look around for a trap and set its type.
 */

void nametrap (int traptype, int standingonit)
{
  register int i, r, c, tdir = NONE, monsteradj = 0;

  if (standingonit)
  {
      r=atrow;
      c=atcol;
  }

  else if (blinded)		/* Cant see, dont bother */
    return;

  else {
    /* Look all around and see what there is next to us */
    for (i = 0; i < 8; i++) {
      r = atdrow(i); c = atdcol(i);

      if (seerc ('^', r, c)) {	/* Aha, a trap! */
        if (tdir != NONE) return;        /* Second trap, ambigous case */
        else tdir = i;                    /* First trap,  record direction */
      }
      else if (isupper(screen[r][c]))   /* Trap could be under monster */
        monsteradj++;
    }

    /* See one trap, set (r,c) to the trap location */
    if (tdir != NONE)
      { r = atdrow(tdir); c =  atdcol(tdir); }

    /* See no traps, if there is a monster adjacent, he could be on it */
    else if (monsteradj)
      return;

    /* Cant ever sit on a trap door or a teleport trap */
    else if (traptype == TRAPDOR || traptype == TELTRAP)
      return;

    /* Cant see trap anywhere else, we must be sitting on it */
    else
      { r = atrow; c = atcol; }
  }

  /* Record last arror trap found (for cheating against 3.6) */
  if (traptype == ARROW) { foundarrowtrap = 1; trapr = r; trapc = c; }
  else if (traptype == TRAPDOR) { foundtrapdoor = 1; }

  /* If a trapdor, reactivate rules */
  if (traptype == TRAPDOR) foundnew ();

  /* Set the trap type */
  unsetrc (TELTRAP|TRAPDOR|BEARTRP|GASTRAP|ARROW|DARTRAP, r, c);
  setrc (TRAP | traptype, r, c);
  debuglog("discover: trap at %d %d\n", r, c);
}

/*
 * findstairs: Look for STAIRS somewhere and set the stairs to that square.
 */

void findstairs (notr, notc)
int notr, notc;
{
  register int r, c;

  stairrow = staircol = NONE;

  for (r = 2; r < (STATUSROW-1); r++)
    for (c = 1; c < (MAXCOLS-1); c++)
      if ((seerc ('%', r, c) || onrc (STAIRS, r, c)) &&
          r != notr && c != notc)
        { setrc (STAIRS, r, c); stairrow = r; staircol = c; }
}

/*
 * downright: Find a square from which we cannot go down or right.
 */

int downright (drow, dcol)
int *drow, *dcol;
{
  register int i=atrow, j=atcol;

  while (i < STATUSROW && j < (MAXCOLS-1)) {
    if (onrc (CANGO, i, j+1)) j++;
    else if (onrc (CANGO, i+1, j)) i++;
    else { *drow = i; *dcol = j; return (1); }
  }

  return (0);
}

/*
 * Try to light up the situation
 */

lightroom ()
{
  int obj;

  /* not in a room nor on door, blinded or room lit?? */
  if ((!on (DOOR | ROOM)) || blinded || !darkroom ())
    return (0);

  if ((obj = havenamed (Scroll, "light")) && (obj >= 0) && reads (obj))
    return (1);

  if ((obj = havewand ("light")) && (obj >= 0) && (itemis (obj, WORTHLESS)))
    return (0);
  else if ((obj = havewand ("light")) && (obj >= 0) && (!itemis (obj, WORTHLESS)))
    return (point (obj, 0));

  return (0);
}

/*
 * darkroom: Are we in a dark room?
 */

darkroom ()
{
  register int dir, dir2, drow, dcol;

  if (!on (DOOR | ROOM))
    return (0);

  for (dir=0; dir<8; dir++)
    if (seerc ('.', (drow = atdrow(dir)), (dcol = atdcol(dir))))
      for (dir2=0; dir2<8; dir2++)
        if (seerc (' ', drow+deltr[dir2], dcol+deltc[dir2]))
          return (1);

  return (0);
}

/*
 * currentrectangle: infer room extent based on clues from walls
 * NOTE: When a door appears on the screen, currentrectangle
 * should be re-initialised.				LGCH
 */

# define fT 1
# define fB 2
# define fL 4
# define fR 8

static int curt, curb, curl, curr;

void currentrectangle ()
{
  int   flags = fT + fB + fL + fR, r, c, any = 1;

  /*
   * DEFINITION: curt is the current top of the room.  This is the
   * topmost row which is known to be a room square.  The wall location
   * is therefore curt-1.  curb: bottom.  curl: left. curr: right
   * Since we discover new info when we step on a square on the
   * extremity of the known room area, the following statement was
   * modified by LGCH to use >=, <= instead of >, <
   */

  if ((atrow >= curb || atrow <= curt || atcol <= curl || atcol >= curr)
      && on (ROOM)) {
    curt = curb = atrow;
    curl = curr = atcol;

    while (any) {
      any = 0;

      if (flags & fT)
        for (r = curt - 1, c = curl - 1; r > 0 && c > 0 && c <= curr + 1; c++)
          if (onrc (ROOM, r, c))      { curt--; any = 1; break; }
          else if (seerc ('-', r, c)) { flags &= ~fT; break; }

      if (flags & fB)
        for (r = curb + 1, c = curl - 1; r > 0 && c > 0 && c <= curr + 1; c++)
          if (onrc (ROOM, r, c))      { curb++; any = 1; break; }
          else if (seerc ('-', r, c)) { flags &= ~fB; break; }

      if (flags & fL)
        for (r = curt, c = curl - 1; r > 0 && c > 0 && r <= curb; r++)
          if (onrc (ROOM, r, c))      { curl--; any = 1; break; }
          else if (seerc ('|', r, c)) { flags &= ~fL; break; }

      if (flags & fR)
        for (r = curt, c = curr + 1; r <= curb; r++)
          if (onrc (ROOM, r, c))      { curr++; any = 1; break; }
          else if (seerc ('|', r, c)) { flags &= ~fR; break; }

    }

    for (r = curt; r <= curb; r++)
      for (c = curl; c <= curr; c++) {
        setrc (ROOM + CANGO, r, c);
        unsetrc	 (HALL, r, c);
      }

# define ckdoor(FLAG, NODOOR, STATIC, INC, S1, S2, I1, I2) \
    if (0 == (flags & FLAG)) \
    { any = 0; \
      if (NODOOR) any = 1; \
      else \
	for (STATIC = S2, INC = I1; INC <= I2; INC++) \
	  if (onrc (DOOR, r, c)) { any = 1; break; } \
      if (any) \
      { for (STATIC = S2, INC = I1; INC <= I2; INC++) \
	  setrc (SEEN+WALL, r, c); \
	for (STATIC = S1, INC = I1; INC <= I2; INC++) \
	  setrc (BOUNDARY, r, c);   /* Room boundary   LGCH */ \
      } \
      else \
      { for (STATIC = S2, INC = I1; INC <= I2; INC++) \
	  setrc (BOUNDARY, r, c);  /* Unseen wall or door LGCH */ \
      } \
    }

    if (curt <= 2) flags &= ~fT;    /* Wall must be on screen edge */

    if (curb >= (STATUSROW-2)) flags &= ~fB;

    if (curl <= 1) flags &= ~fL;

    if (curr >= (MAXCOLS-2)) flags &= ~fR;

    ckdoor (fT, curt<6,  r, c, curt, curt-1, curl-1, curr+1)
    ckdoor (fB, curb>17, r, c, curb, curb+1, curl-1, curr+1)
    ckdoor (fL, curl<STATUSROW, c, r, curl, curl-1, curt-1, curb+1)
    ckdoor (fR, curr>56, c, r, curr, curr+1, curt-1, curb+1)

    /* Fill in the corners of the room without seeing them */
    /* Prevents looking at corners to find missing doors */
    if ((flags & fT+fR) == 0)  setrc (SEEN + WALL, curt-1, curr+1);

    if ((flags & fT+fL) == 0)  setrc (SEEN + WALL, curt-1, curl-1);

    if ((flags & fB+fR) == 0)  setrc (SEEN + WALL, curb+1, curr+1);

    if ((flags & fB+fL) == 0)  setrc (SEEN + WALL, curb+1, curl-1);
  }
}

clearcurrect()
{
  curl = curr = curt = curb = 0;
}

/*
 * updateat: We have moved, record results of our passge...
 *
 * Bug if monster is chasing us:  +######A@
 * Bug if teleported horiz or vert. Infers cango
 */

void updateat ()
{
  register int dr = atrow - atrow0, dc = atcol - atcol0;
  register int i, r, c;
  int   dist, newzone, sum;

  /*
   * Record passage from one zone to the next
   */

  newzone = whichroom (atrow, atcol);

  if (newzone != NONE && zone != NONE && newzone != zone) {
    new_arch = 1;
    zonemap[zone][newzone] = zonemap[newzone][zone] = 1;

    if ((levelmap[zone] & (EXPLORED | HASROOM)) == 0) {
      for (i = 0, sum = 0; i < 9; i++) sum += zonemap[zone][i];

      if (sum >= 3) markexplored (atrow0, atcol0);
    }
  }

  if (newzone != NONE)
    zone = newzone;


  /*
   * Check for teleport, else if we moved multiple squares, mark them as BEEN
   */

  if (direc (dr, dc) != movedir || dr && dc && abs(dr) != abs(dc))
    teleport ();
  else {
    dist = (abs(dr)>abs(dc)) ? abs(dr) : abs(dc);
    dr = (dr > 0) ? 1 : (dr < 0) ? -1 : 0;
    dc = (dc > 0) ? 1 : (dc < 0) ? -1 : 0;

    for (r = atrow0, c = atcol0;
         dist >= 0 && (onrc(DOOR,r,c) || !onrc(WALL,r,c));
         r += dr, c += dc, dist--) {
      setrc (BEEN | SEEN | CANGO, r, c);

      if (!onrc (TRAP, r, c)) setrc (SAFE, r, c);
    }
  }

  /* Mark surrounding area according to what we see */

  if (!on (HALL | DOOR | ROOM) && !blinded) {
    int rr, cc;
    int halls = 0, rooms = 0, rm;
    char *terrain = "nothing";

    for (i=0; i<8; i += 2) {
      rr = atdrow(i); cc = atdcol(i);

      if (onrc (HALL, rr, cc))
        halls++;
      else if (onrc (ROOM, rr, cc))
        rooms++;
    }

    if (seerc ('|', atrow-1, atcol) && seerc ('|', atrow+1, atcol) ||
        seerc ('-', atrow, atcol-1) && seerc ('-', atrow, atcol+1)) {
      set (DOOR | SAFE); unset (HALL | ROOM); terrain = "door";

      if ((rm = whichroom (atrow, atcol)) != NONE) levelmap[rm] |= HASROOM;
    }
    else if (halls > 0)
      { set (HALL | SAFE); unset (DOOR | ROOM); terrain = "hall"; }
    else if (rooms > 0)
      { set (ROOM); unset (HALL | DOOR); terrain = "room"; }
    else
      return;

    dwait (D_INFORM, "Inferring %s at %d,%d.", terrain, atrow, atcol);
  }
  else if (on (DOOR | ROOM) && !isexplored (atrow, atcol) && !darkroom ()) {
    markexplored (atrow, atcol);
  }
}

/*
 * updatepos: Something changed on the screen, update the screen map
 */

void updatepos(char ch, int row, int col)
{
  char  oldch = screen[row][col], *monster, functionchar();
  int   seenbefore = onrc (EVERCLR, row, col);
  int   couldgo = onrc (CANGO, row, col);
  int   unseen = !onrc (SEEN, row, col);
  int   rm = whichroom (row, col);

  debuglog_protocol ("rooms : updatepos (%c, %d, %d)\n",ch, row, col);

  if (mlistlen && ch != oldch)
  {
      deletemonster(row, col); //todo:mdk don't clear held monsters that are still awake
  }

  if (unseen) { foundnew (); }

  switch (ch) {
    case '@':
      setrc (SEEN | CANGO | BEEN | EVERCLR, row, col);
      unsetrc (MONSTER | SLEEPER, row, col);
      atrow = row;
      atcol = col;
      debuglog("player: at %d %d\n", atrow, atcol);
      break;

    case '#':

      if (!onrc (HALL, row, col)) {
        foundnew ();
        timestosearch = k_door / 5;
      }

      if (onrc (STUFF, row, col)) deletestuff (row, col);

      setrc (SEEN | CANGO | SAFE | HALL | EVERCLR, row, col);
      unsetrc (DOOR | ROOM | TRAP | ARROW | TRAPDOR | TELTRAP | GASTRAP |
               BEARTRP | DARTRAP | MONSTER | SCAREM | WALL | SLEEPER | STAIRS,
               row, col);
      break;

    case '+':

      if (!onrc (DOOR, row, col)) {
        foundnew ();
        timestosearch = k_door / 5;
        teleported = 0; /* Dont give up on this level yet */
        *newdoors++ = row;  *newdoors++ = col;
      }

      if (onrc (STUFF, row, col)) deletestuff (row, col);

      setrc (SEEN | CANGO | SAFE | DOOR | WALL | EVERCLR, row, col);
      unsetrc (ROOM | TRAP | ARROW | TRAPDOR | TELTRAP | GASTRAP | BEARTRP |
               DARTRAP | MONSTER | SCAREM | SLEEPER, row, col);
      clearcurrect();  /* LGCH: redo currentrectangle */
      break;

      /*
       * Room floor:  there are many cases of what a room floor means,
       * depending on the version of Rogue, whether the room is lit, whether
       * we are in the room or not, and whether or not we were shooting
       * missiles last turn.
       */

    case '.':
      /* The square cant be any of these */
      unsetrc (HALL | DOOR | MONSTER | SCAREM | WALL | TRAP | ARROW |
               TRAPDOR | TELTRAP | GASTRAP | BEARTRP | DARTRAP, row, col);

      if (!onrc (ROOM, row, col))		/* New room? */
        unmarkexplored (row, col);

      if (rm != NONE) levelmap[rm] |= HASROOM;	/* Room here */

      /* If older Rogue, or our last position or a moving missile or */
      /* in the same room, then a floor '.' means no stuff there     */
      if ((version < RV52A ||
           oldch == '@' ||
           oldch == ')' && functionchar (lastcmd) == 't' ||
           (on (ROOM) && whichroom (row, col) == whichroom (atrow, atcol))) &&
          onrc (STUFF, row, col))
        { deletestuff (row, col); }

      /* If the stairs moved, look for them */
      if (oldch == '@' && onrc (STAIRS, row, col)) findstairs (row, col);

      /* Record whether this square has been clear of monsters */
      if (!isupper (oldch)) setrc (EVERCLR, row, col);

      /* Safe if we have been there, but not if the stuff was an arrow */
      if (onrc (BEEN, row, col)) setrc (SAFE, row, col);
      else if (oldch == ')' && functionchar (lastcmd) == 't')
        unsetrc (SAFE, row, col);

      setrc (SEEN | CANGO | ROOM, row, col);	/* Square must be these */
      break;

    case '-':
    case '|':
      setrc (SEEN | WALL | EVERCLR, row, col);
      unsetrc (CANGO | HALL | DOOR | ROOM | SLEEPER, row, col);
      break;

    case ':':
    case '?':
    case '!':
    case ')':
    case ']':
    case '/':
    case '=':
    case ',':           /* HAH! *//* HAH HAH! *//* HAH HAH HAH! */
    case '*':
      setrc (SEEN | CANGO | SAFE | EVERCLR, row, col);
      unsetrc (DOOR | TRAP | ARROW | TRAPDOR | TELTRAP | GASTRAP | BEARTRP |
               DARTRAP | MONSTER | WALL | SLEEPER, row, col);

      if (ch != '?') unsetrc (SCAREM, row, col);

      if (!onrc (BEEN, row, col) || !onrc(STAIRS, row, col) || !cosmic)
        { addstuff (ch, row, col); unsetrc (STAIRS, row, col); }

      setnewgoal ();
      break;

    case '%':

      if (!onrc (STAIRS, row, col)) foundnew ();

      if ((!cosmic || onrc (BEEN, row, col)) && onrc (STUFF, row, col))
        deletestuff (row, col);

      setrc (SEEN | CANGO | SAFE | ROOM | STAIRS | EVERCLR, row, col);
      unsetrc (DOOR | HALL | TRAP | ARROW | TRAPDOR | TELTRAP | GASTRAP |
               BEARTRP | DARTRAP | MONSTER | SCAREM | SLEEPER,
               row, col);
      stairrow = row;
      staircol = col;
      setnewgoal ();
      break;

    case '^':
      setrc (SEEN | CANGO | ROOM | TRAP | EVERCLR, row, col);

      if (onrc (STUFF, row, col)) deletestuff (row, col);

      unsetrc (SAFE | HALL | DOOR | MONSTER | SCAREM | WALL | SLEEPER,
               row, col);
      break;

    case ' ':
      unsetrc (MONSTER | WALL, row, col);
      break;

    default:

      if (isupper (ch)) {
        monster = monname (ch);
        setrc (SEEN | CANGO | MONSTER, row, col);
        unsetrc (SCAREM, row, col);

        if (onrc (WALL, row, col)) {	/* Infer DOOR here */
          if (!onrc (DOOR, row, col)) {
            foundnew ();
            timestosearch = k_door / 5;
            setrc (DOOR, row, col); /* MLM */
            unsetrc (WALL, row, col); /* MLM */
          }
        }

        if (ch != oldch)
        {
          //blinded = 0; //mdk: removed check to prevent "detect monster" from making us sighted

          //todo:mdk: consider revvideo: monster is either near us in passage, or detected

          if (seenbefore)
            addmonster (ch, row, col, AWAKE);
          else if (!onrc (HALL | DOOR, row, col) && !aggravated &&
                   (streq (monster, "floating eye") ||
                    streq (monster, "ice monster") ||
                    streq (monster, "leprechaun") ||
                    streq (monster, "nymph") ||
                    (version < RV52A && (ch == 'T' || ch == 'P')))) {
            addmonster (ch, row, col, ASLEEP);
            setrc (SLEEPER, row, col);
          }
          else if (onrc (HALL | DOOR, row, col) || aggravated) {
            addmonster (ch, row, col, AWAKE);
            setrc (EVERCLR, row, col);
          }
          else
            addmonster (ch, row, col, 0);
        }
      }

      break;
  }

  /* If the stairs moved, look for the real stairs */
  if ((!onrc (STAIRS, row, col) && (row==stairrow && col==staircol)) ||
      (stairrow != NONE && !onrc (STAIRS, stairrow, staircol)))
    findstairs (row, col);

  if (!couldgo && onrc (CANGO, row, col))
    setnewgoal ();
}

/*
 * teleport: We have just been teleported. Reset whatever is necessary to
 * avoid doing silly things.
 */

void teleport ()
{
  register int r = atrow0, c = atcol0;

  goalr = goalc = NONE; setnewgoal ();

  hitstokill = 0; darkdir = NONE; darkturns = 0;

  if (movedir >= 0 && movedir < 8 && !confused) {
    teleported++;

    while (r > 1 && r < STATUSROW && c > 0 && c < (MAXCOLS-1)) {
      if (onrc (WALL | DOOR | HALL, r, c)) break;

      if (onrc (TRAP, r, c)) {
        if (!onrc (ARROW|DARTRAP|GASTRAP|BEARTRP|TRAPDOR|TELTRAP, r, c))
          saynow ("Assuming teleport trap at %d, %d", r, c);

        break;
      }

      r += deltr[movedir]; c += deltc[movedir];
    }
  }
}

/*
 * mapinfer: Rewritten by Michael Mauldin.  August 19, 1983.
 * Infer bit settings after reading a scroll of magic mapping.
 * Because the mapping scroll gives extra information (in particular
 * we now know all the room squares so we can plan run-away paths
 * properly) it is best to process the entire map making extra
 * inferences.
 */

void mapinfer()
{
  register r, c, inroom;

  dwait (D_CONTROL, "Map read: inferring rooms.");

  for (r=1; r<STATUSROW; r++) {
    inroom = 0;

    for (c=0; c<MAXCOLS; c++) {
      if (seerc ('|', r, c) || (seerc ('+', r, c) && !seerc('-', r, c - 1) && !seerc('-', r, c + 1))) /* mdk:bugfix added +1 check for doors at corners */
        { inroom = !inroom; }
      else if (inroom)
        { setrc (ROOM | CANGO, r, c); }
      else
        { setrc (SEEN, r, c); }
    }
  }
}

/*
 * markexplored: If we are in a room, mark the location as explored.
 */

void markexplored (row, col)
int row, col;
{
  register int rm = whichroom (row, col);

  if (rm != NONE && !(levelmap[rm] & EXPLORED)) {
    levelmap[rm] |= EXPLORED;

    if (!(levelmap[rm] & HASROOM))
      saynow ("Assuming room %d is gone.", zone);
  }
}

/*
 * unmarkexplored: If we are in a room, unmark the location as explored.
 */

void unmarkexplored (row, col)
int row, col;
{
  register int rm = whichroom (row, col);

  if (rm != NONE) levelmap[rm] &= ~EXPLORED;
}

/*
 * isexplored: If we are in a room, return true if it has been explored.
 */

int isexplored (row, col)
int row, col;
{
  register int rm = whichroom (row, col);

  return (rm != NONE ? levelmap[rm] & EXPLORED : 0);
}

/*
 * haveexplored: Have we explored n rooms?
 */

int haveexplored (n)
int n;
{
  register int rm, count = 0;

  for (rm = 0; rm < 9; rm++)
    if (levelmap[rm] & EXPLORED)
      count++;

  return (count >= n);
}

/*
 * printexplored: List the explored rooms
 */

void printexplored ()
{
  register int rm;

  at (0,0);
  printw ("Rooms explored: ");

  for (rm = 0; rm < 9; rm++) {
    if (levelmap[rm] & EXPLORED) {
      printw (" %d", rm);
    }
  }

  clrtoeol ();
  at (row, col);
  refresh ();
}

/*
 * inferhall: When a door appears on the screen where no door was before,
 * check whether we can infer a hall between it an a neighbouring room.
 * The technique is simple: We first determine whether the hall is already
 * known, and if it is not, we scan away from the room looking for another
 * wall. If we find one, then we look for a door and if we find THAT then
 * we infer a hall between the matching doors. Of course, this means that
 * we must set CANGO bits so that exploration can use the guessed hall. So
 * we set CANGO for the complete rectangle joining the two doors and then
 * rely on the CANGO bits being unset again where we actually see blank
 * space.
 */

void inferhall (r, c)
register int r, c;
{
  register int i, j, k;

  int inc, rm, end1, end2, end, dropout = 0, dir = NONE;

  char dirch = ' ';

  for (k = 0; k < 8; k += 2) {
    if (onrc (HALL, r + deltr[k], c + deltc[k]))      /* Hall has been seen */
      return;
    else if (onrc (ROOM, r + deltr[k], c + deltc[k])) /* Room is over here */
      dir = k;
  }

  dwait (D_SEARCH, "Room direction %d", dir);

  if (dir < 0) return;

  if (dir % 4 == 0) {		     /* If horizontal dir */

    if (dir == 0)
      dirch = 'l';
    else
      dirch = 'r';

    inc = -deltc[dir]; rm = whichroom (r, c);
    end1 = bounds[rm].top; end2 = bounds[rm].bot;

    if (inc < 0) end = bounds[rm-1].left;
    else         end = bounds[rm+1].right;

    end = end * inc;

    for (j = c+inc; j*inc < end; j += inc) {
      for (i = end1; i <= end2; i++) {
        if (debug (D_SCREEN | D_SEARCH | D_INFORM)) mvaddch (i, j, dirch);

        if (onrc (DOOR | WALL | ROOM | HALL, i, j)) {
          /* Modified only to find doors on vertical walls */
          if (onrc (DOOR,i,j) && (onrc (WALL,i-1,j) || onrc (WALL,i+1,j)))
            connectdoors (r, c+inc, i, j-inc);

          dropout = 1;
        }
      }

      if (dropout)
        break;
    }
  }

  else {

    if (dir == 2)
      dirch = 'd';
    else
      dirch = 'u';

    inc = -deltr[dir]; rm = whichroom (r, c);
    end1 = bounds[rm].left; end2 = bounds[rm].right;

    if (inc < 0) end = bounds[rm-3].top;
    else         end = bounds[rm+3].bot;

    end = end * inc;

    for (i = r+inc; i*inc < end; i += inc) {
      for (j = end1; j <= end2; j++) {
        if (debug (D_SCREEN | D_SEARCH | D_INFORM)) mvaddch (i, j, dirch);

        if (onrc (DOOR | WALL | ROOM | HALL, i, j)) {
          /* Modified only to find doors on horizontal walls */
          if (onrc (DOOR,i,j) && (onrc (WALL,i,j-1) || onrc (WALL,i,j+1)))
            connectdoors (r+inc, c, i-inc, j);

          dropout = 1;
        }
      }

      if (dropout) break;
    }
  }

  /* NOTE: If we set SEEN here on the three squares beyond the door, then
   * we can prevent Rogomatic's persistence in searching out every
   * corridor that leads to a secret door at the other end. Or, we could set
   * a bit on the door to make it a preferred exploration target so that
   * Rogomatic would ALWAYS search out every corridor leading to a secret
   * door at the other end. The latter alternative is probably better
   * unless we implement the inferred corridors so that we can infer a
   * corridor which has a secret door and therefore we can traverse it
   * more easily one way than the other. NOTE that we must have a flag to
   * indicate why the search for a corridor failed: if it found a wall
   * then we know there is a secret door; if it stopped for another reason
   * then we don't know what we may find - maybe a room, maybe a path to a
   * corridor.
   */

  dwait (D_SEARCH | D_CONTROL, "Hall search done.");
}

void connectdoors (r1, c1, r2, c2)
register int r1, c1, r2, c2;
{
  register int r, c;
  int endr = max (r1, r2), endc = max (c1, c2);

  dwait (D_INFORM, "Inferring hall (%d,%d) to (%d,%d)", r1, c1, r2, c2);

  for (r = min (r1, r2); r <= endr; r++)
    for (c = min (c1, c2); c <= endc; c++)
      setrc (CANGO|SAFE, r, c);              /* Can go (somewhere) here */

  for (r = min (r1, r2) - 1; r <= endr + 1; r++)
    for (c = min (c1, c2) - 1; c <= endc + 1; c++)
      setrc (SEEN, r, c); 		     /* Nothing to see here */
}

/*
 * canbedoor: Called from setpsd() to check that a dead end could in fact
 * lead to a room.  Only checks that there is enough empty space next to a
 * square.  Does NOT check that this square is in fact a dead end.
 *
 * September 25, 1983	Michael L. Mauldin
 */

int canbedoor (deadr, deadc)
int deadr, deadc;
{
  register int r, c, dr, dc, k, count;

  /* Check all orthogonal directions around the square */
  for (k=0; k < 8; k+=2) {
    dr = deltr[k]; dc = deltc[k];
    r = deadr+dr; c = deadc+dc;

    /* If there are four blank squares, then it could be a door */
    for (count=0; count < 4 && seerc (' ',r,c); count++)
      { r+=dr; c+=dc; }

    if (count >= 4) return (1);
  }

  /* Not enough room in any direction */
  return (0);
}

/*
 * mazedoor: Return true if this could be a door to a maze
 */

int mazedoor (row, col)
int row, col;
{
  register int r=row, c=col, dr, dc, k=0, dir = NONE;

  if (onrc (HALL,r,c+1)) {dir=0; k++; dr=0;   dc=1;}

  if (onrc (HALL,r-1,c)) {dir=2; k++; dr= -1; dc=0;}

  if (onrc (HALL,r+1,c)) {dir=6; k++; dr=1;   dc=0;}

  if (onrc (HALL,r,c-1)) {dir=4; k++; dr=0,   dc= -1;}

  if (k != 1) return (0);

  /* Fail if no adjacent hall, or not double corridor */
  if ((onrc (HALL, r+dr+dr, c+dc+dc) == 0))
    return (0);

  /* Must have two sets of double corridor */
  if (! (((onrc (HALL, r+dr+deltr[(dir+1)&7], c+dc+deltc[(dir+1)&7])) &&
          (onrc (HALL, r+dr+deltr[(dir+2)&7], c+dc+deltc[(dir+2)&7]))) ||
         ((onrc (HALL, r+dr+deltr[(dir-1)&7], c+dc+deltc[(dir-1)&7])) &&
          (onrc (HALL, r+dr+deltr[(dir-2)&7], c+dc+deltc[(dir-2)&7])))))
    return (0);

  /* If there are four blank squares, then it could be a door */
  for (r = row-dr, c = col-dc, k=0;  k < 4 && seerc (' ',r,c);  k++)
    { r-=dr; c-=dc; }

  if (k >= 4) return (1);

  /* Not enough room for room */
  return (0);
}

/*
 * nextto:  Is there a square type orthogonally adjacent?
 */

int nextto (type,r,c)
register int type, r, c;
{
  register int result;

  if (result = onrc (type, r-1, c)) return (result);

  if (result = onrc (type, r+1, c)) return (result);

  if (result = onrc (type, r, c-1)) return (result);

  if (result = onrc (type, r, c+1)) return (result);

  return (0);
}

/*
 * nexttowall:  Is there a wall adjacent wall?
 *			|
 *  e.g.	########|   <----   there should be a door here.
 *			|
 * Fuzzy:	Replaces knowisdoor (), October 17, 1983.
 */

int nexttowall (r,c)
register int r, c;
{
  return (onrc (DOOR | WALL, r-1, c) == WALL ||
          onrc (DOOR | WALL, r+1, c) == WALL ||
          onrc (DOOR | WALL, r, c-1) == WALL ||
          onrc (DOOR | WALL, r, c+1) == WALL);
}

/*
 * dumpmazedoor: Show all squares for which mazedoor(r,c) is true.
 */

void dumpmazedoor ()
{
  register int r, c;

  for (r=2; r<(STATUSROW-1); r++) {
    for (c=1; c<(MAXCOLS-1); c++) {
      if (((scrmap[r][c] & (BEEN|DOOR|HALL|ROOM|WALL|STAIRS)) == 0) &&
          mazedoor (r, c))
        mvaddch (r, c, 'M');
    }
  }

  at (row, col);
}

/*
 * foundnew: Reactivate rules which new new squares to work
 */

void foundnew ()
{
  new_mark = new_findroom = new_search = new_stairs = 1;
  reusepsd = teleported = 0;
  cancelmove (SECRETDOOR);
  unrest ();
}
