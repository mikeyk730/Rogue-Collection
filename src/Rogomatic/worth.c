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
 * worth.c:
 *
 * This file contains the function worth (obj) which does the impossible
 * job of deciding how much each item in the pack is worth.
 *
 * The worth of an item is a number from 0 to 5000, with 0 indicating that
 * the object is completely useless, and 5000 indicating that its a really
 * nifty piece of work.  This function is used by 'tostuff' to decide
 * which is the "worst" object in the pack.
 */

# include <curses.h>
# include "types.h"
# include "globals.h"

int   objval[] = {
  /* strange */      0,
  /* food */       900,
  /* potion */     500,
  /* scroll */     400,
  /* wand */       600,
  /* ring */       800,
  /* hitter */     100,
  /* thrower */    100,
  /* missile */    300,
  /* armor */      200,
  /* amulet */    5000,
  /* gold */      1000,
  /* none */         0
};

int worth (obj)
int obj;
{
  int value, w;

  /* Do we have an easy out? */
  if (useless (obj)) return (0);

  /* Poison has a use in RV52B and RV53A, so give it a low positive value */
  if (stlmatch (inven[obj].str, "poison")) return (1);

  /* Set base value */
  value = objval[(int) inven[obj].type];

  /* Bonus if we know what it is */
  if (itemis (obj, KNOWN))
    value += 50;

  /*
   * Armor values are based on armor class, bonus for best, second
   * best, third best, or leather armor (leather doesnt rust)
   */

  if (inven[obj].type == armor) {
    value = (11 - armorclass (obj)) * 120;

    if (obj == havearmor (1, NOPRINT, ANY))		value += 2000;
    else if (obj == havearmor (2, NOPRINT, ANY))	value += 1500;
    else if (obj == havearmor (3, NOPRINT, ANY))	value += 800;

    if (stlmatch (inven[obj].str, "leather"))		value += 300;
  }

  /*
   * Bow values are based on bow class, bonus for best
   * or second best.
   */

  else if (inven[obj].type == thrower) {
    value = (bowclass (obj));

    if (obj == havebow (1, NOPRINT)) value += 1500;
    else if (obj == havebow (2, NOPRINT)) value += 300;
  }

  /* Weapons values are counted by hit potential, bonus for best */
  else if ((w = weaponclass (obj)) > 0) {
    value = w * 5;

    if (obj == haveweapon (1, NOPRINT)) value += 2500;
    else if (obj == haveweapon (2, NOPRINT)) value += 1500;
  }

  /* Rings values are counted by bonus */
  else if ((w = ringclass (obj)) > 0) {
    if (w > 1000) w -= 500; /* Subtract part of food bonus */

    value = w + 400;
  }

  /* For arbitrary things, bonus for plus item */
  else {
    if (inven[obj].phit != UNKNOWN)
      value += inven[obj].phit * 75;
  }

  /* Prefer larger bundles of missiles */
  if (inven[obj].type == missile)
    value += inven[obj].count * 50;

  /* Prefer wands with more charges */
  if (inven[obj].type == wand && inven[obj].charges != UNKNOWN)
    value += inven[obj].charges * 35;

  /* Special Values for Certain objects */
  if (stlmatch (inven[obj].str, "raise level")) value = 1200;
  else if (stlmatch (inven[obj].str, "restore strength")) value = 800;
  else if (stlmatch (inven[obj].str, "gain strength")) value = 700;
  else if (stlmatch (inven[obj].str, "scare monster")) value = 1400;
  else if (stlmatch (inven[obj].str, "teleportation")) value = 1000;
  else if (stlmatch (inven[obj].str, "enchant")) value = 800;
  else if (stlmatch (inven[obj].str, "extra healing")) value = 900;
  else if (stlmatch (inven[obj].str, "healing")) value = 750;
  else if (stlmatch (inven[obj].str, "protect") && !protected) value = 1000;

  /* Now return the value, assure in the range [0..5000] */
  return (value < 0 ? 0 : value > 5000 ? 5000 : value);
}

/*
 * useless: called with an integer from 0 to 25 it returns 1 if that
 * object is of no use. Used by worth to set value to 0.
 */

int useless (i)
int i;
{
  /* Not useless if we are using it */
  if (itemis (i, INUSE))
    return (0);

  /* item has been tagged WORTHLESS someplace else */
  if (itemis (i, WORTHLESS))
    return (1);

  /* Worn out or bad wands are useless */
  if ((inven[i].type == wand) &&
      (inven[i].charges == 0 ||
       stlmatch (inven[i].str, "teleport to") ||
       stlmatch (inven[i].str, "haste monster")))
    return (1);

  /* Many potions are useless */
  if (inven[i].type == potion && itemis (i, KNOWN) &&
      (stlmatch (inven[i].str, "paralysi") ||
       stlmatch (inven[i].str, "confusion") ||
       stlmatch (inven[i].str, "hallucination") ||
       stlmatch (inven[i].str, "blind") ||
       stlmatch (inven[i].str, "monster detection") ||
       stlmatch (inven[i].str, "magic detection") ||
       stlmatch (inven[i].str, "thirst") ||
       (stlmatch (inven[i].str, "haste self") && doublehasted) ||
       (stlmatch (inven[i].str, "see invisible") &&
        havenamed (ring, "see invisible") != NONE)))
    return (1);

  /* So are many scrolls */
  if (inven[i].type == Scroll && itemis (i, KNOWN) &&
      (stlmatch (inven[i].str, "blank") ||
       stlmatch (inven[i].str, "create monster") ||
       stlmatch (inven[i].str, "sleep") ||
       stlmatch (inven[i].str, "gold detection") ||
       stlmatch (inven[i].str, "aggravate")))
    return (1);

  /* And bad rings are useless */
  if (inven[i].type == ring && itemis (i, KNOWN) &&
      ((inven[i].phit != UNKNOWN && inven[i].phit < 0) ||
       stlmatch (inven[i].str, "teleport") ||
       stlmatch (inven[i].str, "telport") ||     /* For R3.6 MLM */
       stlmatch (inven[i].str, "adornment") ||
       stlmatch (inven[i].str, "aggravate")))
    return (1);

  /* One of some rings is enough */
  if (inven[i].type == ring && itemis (i, KNOWN) &&
      havemult (ring, inven[i].str, 2) != NONE &&
      (stlmatch (inven[i].str, "see invisible") ||
       stlmatch (inven[i].str, "sustain strength") ||
       stlmatch (inven[i].str, "searching") ||
       stlmatch (inven[i].str, "increase damage") ||
       stlmatch (inven[i].str, "stealth")))
    return (1);

  /* Three of any ring is too much */
  if (inven[i].type == ring && havemult (ring, inven[i].str, 3) != NONE)
    return (1);

  /* If we are cheating and we have a good arrow */
  /* then many rings do us no good at all.       */
  if (inven[i].type == ring && usingarrow && goodarrow > 20 &&
      (stlmatch (inven[i].str, "add strength") ||
       stlmatch (inven[i].str, "dexterity") ||
       stlmatch (inven[i].str, "increase damage")))
    return (1);

  return (0);
}
