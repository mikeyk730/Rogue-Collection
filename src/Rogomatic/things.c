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
 * things.c:
 *
 * This file contains much of the code to handle Rog-O-Matics inventory.
 */

# include <ctype.h>
# include <curses.h>
# include <string.h>
# include "types.h"
# include "globals.h"

/*
 * wear: This primitive function issues a command to put on armor.
 */

wear (obj)
int obj;
{
  if (currentarmor != NONE) {
    dwait (D_FATAL, "Trying to put on a second coat of armor");
    return (0);
  }

  if (cursedarmor) return (0);

  command (T_HANDLING, "W%c", LETTER (obj));
  usesynch = 0;
  return (1);
}

/*
 * takeoff: Remove the current armor.
 */

int takeoff ()
{
  if (currentarmor == NONE) {
    dwait (D_ERROR, "Trying to take off armor we don't have on!");
    return (0);
  }

  if (cursedarmor) return (0);

  command (T_HANDLING, "T");
  usesynch = 0;
  return (1);
}

/*
 * wield: This primitive function issues a command to wield a weapon.
 */

int wield (obj)
int obj;
{
  if (cursedweapon) return (0);

  if (new_weapon_protocol()) {

    if (itemis(currentweapon, CURSED)) {
      cursedweapon=1;
      return (0);
    }
    else if (currentweapon == NONE) {
      command (T_HANDLING, "w%c", LETTER (obj));
    }
    else if (itemis(currentweapon, UNCURSED)) {
      cursedweapon=0;
      command (T_HANDLING, "w%c", LETTER (obj));
    }
    else if (itemis(currentweapon, ENCHANTED)) {
      remember(currentweapon, UNCURSED);
      cursedweapon=0;
      command (T_HANDLING, "w%c", LETTER (obj));
    }
    else {

      /* current weapon might be cursed */
      if (currentweapon != NONE) {
        lastdrop = currentweapon;
        command (T_HANDLING, "w%c", ESC);
      }
    return (0);
    }
  }

  /* send 2 escapes because I needed to patch the new rogue to not hang
   * momentatirily on the first escape
   */
  else
    command (T_HANDLING, "w%cw%c%c", LETTER (obj), ESC, get_repeat_message_key());

  return (1);
}

/*
 * destroyjunk: When an object is thrown diagonally into a corner,
 *           Rogue can't find a place to put it, and the object is
 *           removed from the game (adapted from dropjunk).
 */

destroyjunk (obj)
int obj;
{

  if ((obj != NONE) && (gotocorner () || throw (obj, 7)))
    return (1);

  return (0);
}

/*
 * drop: called with an integer from 0 to 25, drops the object if possible
 * and returns 1 if it wins and 0 if it fails.
 */

int drop (obj)
int obj;
{
  /* Can't if not there, in use, or on something else or
     dropped something else already */
  if (inven[obj].count < 1 ||
      itemis (obj, INUSE) ||
      on (STUFF | TRAP | STAIRS | DOOR) ||
      diddrop)
    return (0);

  if ((obj != NONE) && (inven[obj].type == wand))
    return (destroyjunk (obj));

  /* read unknown scrolls or good scrolls rather than dropping them */
  if (inven[obj].type == Scroll &&
      ((!itemis (obj, KNOWN) && prepareident(pickident(), obj)) || /* mdk:bugfix: added prepareident in case the unknown scroll is an identify scroll */
       stlmatch (inven[obj].str, "identify") &&
       prepareident (pickident (), obj) ||
       stlmatch (inven[obj].str, "enchant") ||
       stlmatch (inven[obj].str, "genocide") ||
       stlmatch (inven[obj].str, "gold detection") ||
       stlmatch (inven[obj].str, "hold monster") ||
       stlmatch (inven[obj].str, "light") ||
       stlmatch (inven[obj].str, "magic mapping") ||
       stlmatch (inven[obj].str, "monster confusion") ||
       stlmatch (inven[obj].str, "remove curse")) &&
      reads (obj))
    { return (1); }

  /* quaff unknown potions or good potions rather than dropping them */
  if (inven[obj].type == potion &&
      (!itemis (obj, KNOWN) ||
       stlmatch (inven[obj].str, "magic detection") ||
       stlmatch (inven[obj].str, "monster detection") ||
       stlmatch (inven[obj].str, "raise level") ||
       stlmatch (inven[obj].str, "healing") ||
       stlmatch (inven[obj].str, "haste self") && !hasted ||
       stlmatch (inven[obj].str, "extra healing") ||
       stlmatch (inven[obj].str, "restore strength") ||
       stlmatch (inven[obj].str, "gain strength")) &&
      quaff (obj))
    { return (1); }

  command (T_HANDLING, "d%c", LETTER (obj));
  return (1);
}

/*
 * quaff: build and send a quaff potion command.
 */

int quaff (obj)
int obj;
{
  if (inven[obj].type != potion) {
    dwait (D_ERROR, "Trying to quaff %c", LETTER (obj));
    usesynch = 0;
    return (0);
  }

  command (T_HANDLING, "q%c", LETTER (obj));
  return (1);
}

/*
 * reads: build and send a read scroll command.
 */

int reads (obj)
int obj;
{
  if (inven[obj].type != Scroll) {
    dwait (D_ERROR, "Trying to read %c", LETTER (obj));
    usesynch = 0;
    return (0);
  }

  command (T_HANDLING, "r%c", LETTER (obj));
  return (1);
}

/*
 * build and send a point with wand command.
 */

int point (obj, dir)
int obj, dir;
{
  if (inven[obj].type != wand) {
    dwait (D_ERROR, "Trying to point %c", LETTER (obj));
    return (0);
  }

  if (itemis (obj, USELESS))
    return (0);
  else {
    command (T_HANDLING, "%c%c%c",
             get_zap_key(),	/* R5.2 MLM */
             keydir[dir], LETTER (obj));
    return (1);
  }
}

/*
 * throw: build and send a throw object command.
 */

int throw (obj, dir)
int obj, dir;
{
  if (obj < 0 || obj >= invcount) {
    dwait (D_ERROR, "Trying to throw %c", LETTER (obj));
    return (0);
  }

  command (T_HANDLING, "t%c%c", keydir[dir], LETTER (obj));
  return (1);
}

/*
 * puton: build and send a command to put on a ring.
 */

int puton (obj)
int obj;
{
  if (leftring == NONE && rightring == NONE)
    { command (T_HANDLING, "P%cl", LETTER (obj)); return (1); }

  if (leftring == NONE || rightring == NONE)
    { command (T_HANDLING, "P%c", LETTER (obj)); return (1); }

  return (0);
}

/*
 * removering: build a command to remove a ring. It is left in the pack.
 */

int removering (obj)
int obj;
{
  if (leftring != NONE && rightring != NONE && leftring == obj)
    { command (T_HANDLING, "Rl"); return (1); }

  if (leftring != NONE && rightring != NONE && rightring == obj)
    { command (T_HANDLING, "Rr"); return (1); }

  if (leftring == obj || rightring == obj)
    { command (T_HANDLING, "R"); return (1); }

  return (0);
}

/*
 * initstufflist: clear the list of objects on this level.
 */

void initstufflist ()
{
  slistlen = 0;
}

/*
 * addstuff: add an item to the list of items on this level.
 */

void addstuff (ch, row, col)
char  ch;
int   row, col;
{
  /* if (seerc ('@', row, col)) return (0); */ /* Removed MLM 10/28/83 */
  if (onrc (STUFF, row, col))
    deletestuff (row, col);

  slist[slistlen].what = translate[ch];
  slist[slistlen].srow = row;
  slist[slistlen].scol = col;

  if (++slistlen >= MAXSTUFF) dwait (D_FATAL, "Too much stuff");

  setrc (STUFF, row, col);
}

/*
 * deletestuff: remove the object from the stuff list at location (x,y)
 */

void deletestuff (row, col)
int   row, col;
{
  register int   i;
  unsetrc (STUFF, row, col);

  for (i = 0; i < slistlen; ++i)
    if (slist[i].scol == col && slist[i].srow == row) {
      slist[i] = slist[--slistlen];
      i--;					/* MLM 10/23/82 */
    }
}

/*
 * dumpstuff: (debugging) dump the list of objects on this level.
 */

void dumpstuff ()
{
  register int   i;
  at (1, 0);

  for (i = 0; i < slistlen; ++i)
    printw ("%d at %d,%d (%c)\n",
            slist[i].what, slist[i].srow, slist[i].scol,
            screen[slist[i].srow][slist[i].scol]);

  printw ("You are at %d,%d.", atrow, atcol);
  at (row, col);
}

/*
 * display: Print a message on line 1 of the screen.
 */
void display (s)
char *s;
{
  saynow (s);
  msgonscreen=1;
}

/*
 * prepareident: Set nextid and afterid to proper values
 */

int prepareident (obj, iscroll)
int obj, iscroll;
{
  nextid = LETTER (obj);
  afterid = (iscroll > obj || inven[iscroll].count > 1) ? nextid : nextid-1;
  return (nextid >= 'a' && afterid >= 'a');
}

/*
 * pickident: Pick an object to be identified.  This is a preference
 * ordering of objects.  If nothing else, return 0 (the index of the
 * first item in the pack).
 */

int pickident ()
{
  register int obj;

  if      ((obj=unknown      (ring))   != NONE);
  else if ((obj=unidentified (wand))   != NONE);
  else if ((obj=unidentified (Scroll)) != NONE);
  else if ((obj=unidentified (potion)) != NONE);
  else if ((obj=unknown      (Scroll)) != NONE);
  else if ((obj=unknown      (potion)) != NONE);
  else if ((obj=unknown      (hitter)) != NONE);
  else obj = 0;

  return (obj);
}

/*
 * unknown: Return the index of any unknown object of type otype
 */

int unknown (otype)
stuff otype;
{
  register int i;

  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        (inven[i].type == otype) &&
        (itemis (i, KNOWN) == 0))
      return (i);

  return (NONE);
}

/*
 * unidentified: Return the index of any unidentified object of type otype
 */

int unidentified (otype)
stuff otype;
{
  register int i;

  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        (inven[i].type == otype) &&
        (itemis (i, KNOWN) == 0) &&
        (used (inven[i].str)))
      return (i);

  return (NONE);
}

/*
 * haveother: Return the index of any unknown object of type 'otype',
 * but not 'other'.
 */

int haveother (otype,other)
stuff otype;
int other;
{
  register int i;

  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        (inven[i].type == otype) &&
        (itemis (i, KNOWN) == 0) &&
        (i != other))
      return (i);

  return (NONE);
}

/*
 * have: Return the index of any object of type otype
 */

int have (otype)
stuff otype;
{
  register int i;

  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        inven[i].type == otype) return (i);

  return (NONE);
}

/*
 * havenamed: Return the index of any object of type otype named
 * name which is not in use .
 */

int havenamed (otype,name)
stuff otype;
char *name;
{
  register int i;

  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        inven[i].type == otype &&
        (*name == 0 || streq (inven[i].str,name)) &&
        !itemis (i, INUSE))
      return (i);

  return (NONE);
}

/*
 * havewand: Return the index of a charged wand or staff
 */

int havewand (name)
char *name;
{
  register int i;

  /* Find one with positive charges */
  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        inven[i].type == wand &&
        (*name == 0 || streq (inven[i].str,name)) &&
        (inven[i].charges > 0))
      return (i);

  /* Find one with unknown charges */
  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        inven[i].type == wand &&
        (*name == 0 || streq (inven[i].str,name)) &&
        inven[i].charges == UNKNOWN)
      return (i);

  return (NONE);
}

/*
 * wearing: Return the index if wearing a ring with this title
 */

int wearing (name)
char *name;
{
  register int result = NONE;

  if (leftring != NONE && itemis (leftring, INUSE) &&
      streq (inven[leftring].str, name))
    result = leftring;

  else if (rightring != NONE && itemis (rightring, INUSE) &&
           streq (inven[rightring].str, name))
    result = rightring;

  return (result);
}

/*
 * Return the index of any object of type otype and name name only
 * if we have count or more of them. This way we can avoid using the
 * last of something .
 */

int havemult (otype, name, count)
stuff otype;
char *name;
int   count;
{
  register int i, num=count;

  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        inven[i].type == otype &&
        (*name == 0 || streq (inven[i].str,name)) &&
        (num -= inven[i].count) <= 0)
      return (i);

  return (NONE);
}

/*
 * haveminus: Return the index of something if it is a minus item
 * (used to throw away stuff at end)
 */

int haveminus ()
{
  register int i;

  for (i=0; i<invcount; ++i)
    if (inven[i].count &&
        inven[i].phit != UNKNOWN &&
        inven[i].phit < 0)
      return (i);

  return (NONE);
}

/*
 * haveuseless: return the index of useless stuff, in order by:
 *          - empty wands/staffs
 *          - worthless arrows
 *          - yucky potions
 *          - nasty scrolls
 *          - tarnished rings
 *          - dented armor
 *          - dull weapons
 *          - stringless bows
 *
 *   Below level 15 we leave armor, weapons and bows alone, but
 *    after level 14 we only leave the top two of each alone...
 *
 *   and if all those fail return NONE
 */

int haveuseless ()
{
  register int i;

  for (i=0; i<invcount; ++i) {
    if (inven[i].count > 0) {
      if (inven[i].type == wand && inven[i].charges == 0 ||
          stlmatch (inven[i].str, "teleport to") ||
          stlmatch (inven[i].str, "haste monster") ||
          itemis (i, WORTHLESS) && streq (inven[i].str, "arrow"))
        return (i);
      else if (inven[i].type == potion &&
        (stlmatch (inven[i].str, "blindness") ||
         stlmatch (inven[i].str, "poison") ||
         stlmatch (inven[i].str, "confusion") ||
         stlmatch (inven[i].str, "magic detection") ||
         stlmatch (inven[i].str, "paralysis") ||
         stlmatch (inven[i].str, "hallucination") ||
         stlmatch (inven[i].str, "thirst") ||
         stlmatch (inven[i].str, "food detection") ||
         stlmatch (inven[i].str, "monster detection")))
        return (i);
      else if (inven[i].type == Scroll &&
        (stlmatch (inven[i].str, "sleep") ||
         stlmatch (inven[i].str, "blank") ||
         stlmatch (inven[i].str, "create monster") ||
         stlmatch (inven[i].str, "gold detection") ||
         stlmatch (inven[i].str, "aggravate monsters")))
        return (i);
      else if (!itemis (i, INUSE) && itemis (i, KNOWN) &&
        (inven[i].type == missile) &&
        (((inven[i].phit != UNKNOWN) && (inven[i].phit < 0)) ||
         ((inven[i].pdam != UNKNOWN) && (inven[i].pdam < 0))))
        return (i);
      else if (inven[i].type == ring &&
        (stlmatch (inven[i].str, "teleport") ||
         stlmatch (inven[i].str, "adornment") ||
         stlmatch (inven[i].str, "aggravate monster")))
        return (i);
    }
  }
  if (Level > 14) {
    if (((i = havearmor (3, NOPRINT, ANY)) != NONE) && (!itemis (i, INUSE)))
      return (i);
    else if (((i = haveweapon (3, NOPRINT)) != NONE) && (!itemis (i, INUSE)))
      return (i);
    else if (((i = havebow (3, NOPRINT)) != NONE) && (!itemis (i, INUSE)))
      return (i);
  }

  return (NONE);
}

/*
 * willrust: return true if a suit of armor can rust
 */

int willrust (obj)
int obj;
{
  return (! (protected ||
             armorclass (obj) > 8 || armorclass (obj) < -5 ||
             itemis (obj, PROTECTED) ||
             stlmatch (inven[obj].str, "leather") && leather_is_rustproof()));
}

/*
 * wielding: return true if we are wielding an object of type 'otype'
 */

int wielding (otype)
stuff otype;
{
  return (inven[currentweapon].type == otype);
}

/*
 * hungry: return true if we are hungry, weak, or fainting
 */

hungry ()
{ return (*Ms == 'H' || *Ms == 'W' || *Ms == 'F'); }

/*
 * weak: return true if we are weak or fainting
 */

weak ()
{ return (*Ms == 'W' || *Ms == 'F'); }

/*
 * fainting: return true if we are fainting
 */

fainting ()
{ return (*Ms == 'F'); }

/*
 * havefood: return true if we have more than 'n' foods, modified
 * by the genetic variable k_food (higher values of k_food mean this
 * routine returns true less often).
 */

int havefood (n)
int n;
{
  int remaining, foodest, desired;

  if (hungry () || weak () || fainting ())
    return (0);

  remaining = 800 - turns + lastate;

  if (remaining < 0) remaining = 0;

  foodest = larder * 1000 + remaining;
  desired = n * 1000 * 50 / (100-k_food);

  return (foodest > desired);
}
