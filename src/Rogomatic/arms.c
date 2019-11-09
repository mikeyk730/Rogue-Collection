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
 * arms.c:
 *
 * This file contains all utility functions which exist for armor,
 * weapons and rings.
 */

# include <curses.h>
# include "types.h"
# include "globals.h"

/*
 * havearmor: Return Kth best armor. K should be in the range 1 to invcount.
 *            If K is greater than the number of pieces of armor we have,
 *            then NONE is returned.  Will not return cursed armor or
 *	      armor worse than wearing nothing.
 */

# define swap(x,y) {t=(x); (x)=(y); (y)=t;}

int havearmor (k, print, rustproof)
{
  register int i, j, w, t, n=0;
  int armind[MAXINV], armval[MAXINV];

  /* Sort armor by armor class (best first) */
  for (i=0; i<invcount; ++i) {
    if (inven[i].count && inven[i].type == armor &&
        ! (rustproof && willrust (i))) {
      n++;
      w = armorclass (i);

      for (j = n-1; j > 0 && w <= armval[j-1]; j--) {
        swap (armind[j], armind[j-1]);
        swap (armval[j], armval[j-1]);
      }

      armind[j] = i;
      armval[j] = w;
    }
  }

  if (print) {
    mvprintw (1,0, "Current %sArmor Rankings", rustproof ? "Rustproof " : "");

    for (i = 0; i<n; i++)
      mvprintw (i+3, 8, "%2d: %3d %s", i+1, armval[i], itemstr (armind[i]));
  }

  return ((k <= n) ? armind[k-1] : NONE);
}

/*
 * armorclass: Given the index of a piece of armor, return the armor
 * class. Assume modifiers of +2 for unknown armor when we have scrolls
 * of remove curse and -2 for when we don't have a remove curse.
 */

int armorclass (i)
int i;
{
  int class;

  if (inven[i].type != armor)
    return (1000);

  if (stlmatch (inven[i].str, "leather"))        class = 8;
  else if (stlmatch (inven[i].str, "ring"))      class = 7;
  else if (stlmatch (inven[i].str, "studded"))   class = 7;
  else if (stlmatch (inven[i].str, "scale"))     class = 6;
  else if (stlmatch (inven[i].str, "chain"))     class = 5;
  else if (stlmatch (inven[i].str, "splint"))    class = 4;
  else if (stlmatch (inven[i].str, "banded"))    class = 4;
  else if (stlmatch (inven[i].str, "plate"))     class = 3;
  else                                           class = 1000;

  /* Know the modifier exactly */
  if (inven[i].phit != UNKNOWN)
    class -= inven[i].phit;

  /* Can remove curse, so assume it is +2 armor */
  else if (havenamed (Scroll, "remove curse") != NONE)
    class -= 2;

  /* Can't remove curse, assume it is -2 armor */
  else
    class += 2;

  return (class);
}

/*
 * haveweapon: Return Kth best weapon. K should be in the range 1 to invcount.
 *             If K is greater than the number of weapons we have,
 *             then NONE is returned.
 */

int haveweapon (k, print)
{
  register int i, j, w, t, n=0;
  int weapind[MAXINV], weapval[MAXINV];

  for (i=0; i<invcount; ++i)
    if (inven[i].count && (w = weaponclass (i)) > 0) {
      n++;

      for (j = n-1; j > 0 && w >= weapval[j-1]; j--) {
        swap (weapind[j], weapind[j-1]);
        swap (weapval[j], weapval[j-1]);
      }

      weapind[j] = i;
      weapval[j] = w;
    }

  /*
   * Put enchanted weapons above unenchanted ones if the weapon
   * ratings are equal.  DR UTexas 25 Jan 84
   */

  for (j = n-1; j > 0; j--) {
    if (weapval[j] == weapval[j-1]) {
      i = weapind[j];
      w = weapind[j-1];

      if (!itemis (w, ENCHANTED) && itemis (i, ENCHANTED) &&
          !itemis (w, KNOWN) && !itemis (i, KNOWN))
        { swap (weapind[j], weapind[j-1]); }
    }
  }

  if (print) {
    mvaddstr (1, 0, "Current Weapon Rankings");

    for (i = 0; i<n; i++)
      mvprintw (i+3, 8, "%2d: %5d %s", i+1, weapval[i], itemstr (weapind[i]));
  }

  return ((k <= n) ? weapind[k-1] : NONE);
}

/*
 * weaponclass: Given the index of a weapon, return the weapon class.
 *              This is the average damage done + 3/2 the plus to
 *              hit, multiplied by 10. Magic arrows are given very
 *              high numbers.
 */

weaponclass (i)
int i;
{
  int class, hitplus = 0, damplus = 0;

  /* Swords and maces are always valid weapons */
  if (inven[i].type == hitter)
    ;
  /* Under special circumstances, arrows are valid weapons (Hee hee) */
  else if (cheat && inven[i].type == missile &&
           stlmatch (inven[i].str, "arrow"))
    ;
  /* Not a valid weapon */
  else
    return (0);

  /*
   * Set the basic expected damage done by the weapon.
   */

  if (stlmatch (inven[i].str, "mace"))
    class =  50;
  else if (stlmatch (inven[i].str, "two handed sword"))
    class = (version_uses_old_weapon_stats()) ? 105 : 100;
  else if (stlmatch (inven[i].str, "long sword"))
    class = (version_uses_old_weapon_stats()) ? 55 : 75;
  else if (stlmatch (inven[i].str, "arrow"))
    class =  10;
  else
    class =   0;

  /* Know the modifier exactly */
  if (inven[i].phit != UNKNOWN) {
    hitplus += inven[i].phit;

    if (inven[i].pdam != UNKNOWN)
      damplus = inven[i].pdam;
  }

  /*
   * Strategy for "Magic Arrows". These are single arrows when
   * we are cheating. Since arrows normally come in clumps, and
   * since we have never (in cheat mode) thrown any, then a
   * single arrow must have come from a trap, and until it fails
   * to kill something, we assume it is a valuable arrow.
   */

  else if (cheat && version_has_arrow_bug() && usingarrow && goodarrow > 20 &&
           i == currentweapon)
    return (1800);

  else if (cheat && version_has_arrow_bug() && stlmatch (inven[i].str, "arrow") &&
           inven[i].count == 1 && !itemis (i, WORTHLESS) &&
           (!badarrow || i != currentweapon))
    { hitplus = 50;  damplus = 50; }

  hitplus = (hitplus < 100) ? hitplus : 100;
  damplus = (damplus < 200) ? damplus : 200;

  return (class + 12*hitplus + 10*damplus);
}

/*
 * havering: Return Kth best ring. K should be in the range 1 to invcount.
 *           If K is greater than the number of rings we have,
 *           then NONE is returned.
 */

int havering (k, print)
{
  register int i, j, r, t, n=0;
  int ringind[MAXINV], ringval[MAXINV];

  for (i=0; i<invcount; ++i)
    if (inven[i].count && (r = ringclass (i)) > 0) {
      n++;

      for (j = n-1; j > 0 && r >= ringval[j-1]; j--) {
        swap (ringind[j], ringind[j-1]);
        swap (ringval[j], ringval[j-1]);
      }

      ringind[j] = i;
      ringval[j] = r;
    }

  if (print) {
    mvaddstr (1, 0, "Current Ring Rankings");

    for (i = 0; i<n; i++)
      mvprintw (i+3, 8, "%2d: %5d  %s", i+1, ringval[i], itemstr (ringind[i]));
  }

  /*
   * Since rings are class [1-1000] if we don't want to wear them,
   * return the ring index only if its value is greater than 1000.
   */

  return ((k <= n && ringval[k-1] > 1000) ? ringind[k-1] : NONE);
}

/*
 * ringclass: Given the index of a ring, return the ring class.
 *            This is a subjective measure of how much good it
 *            would do us to wear this ring. Values of [1-1000] indicates
 *            that we should not wear this ring at all. A value of 0
 *            indicates a worthless ring. This routine uses the amount
 *            of food available to decide how valuable rings are.
 *            Worth evaluates the value of a ring by subtracting 1000 if
 *            the ringclass is over 1000 to decide how valuable the ring
 *            is, so we add 1000 to indicate that the ring should be worn
 *            and try to assign values from 0 to 999 to determine the
 *            value of the ring.
 */

int ringclass (i)
int i;
{
  int class = 0, magicplus = 0;

  if (inven[i].type != ring)
    return (0);

  /* Get the magic plus */
  if (inven[i].phit != UNKNOWN) {
    magicplus = inven[i].phit;
  }

  /* A ring of protection */
  if (stlmatch (inven[i].str, "protection"))
    { if (magicplus > 0) class = (havefood (1) ? 1000 : 0) + 450; }

  /* A ring of add strength */
  else if (stlmatch (inven[i].str, "add strength")) {
    if (itemis (i, INUSE) && magicplus > 0) {
      if (hitbonus (Str) == hitbonus (Str - magicplus * 100) &&
          damagebonus (Str) == damagebonus (Str - magicplus * 100))
        class = 400;
      else
        class = (havefood (1) ? 1000 : 0) + 400;
    }
    else if (magicplus > 0) {
      if (hitbonus (Str) == hitbonus (Str + magicplus * 100) &&
          damagebonus (Str) == damagebonus (Str + magicplus * 100))
        class = 400;
      else
        class = (havefood (1) ? 1000 : 0) + 400;
    }
  }

  /* A ring of sustain strength */
  else if (stlmatch (inven[i].str, "sustain strength")) {
    /* A second ring of sustain strength is useless */
    if (!itemis (i, INUSE) && wearing ("sustain strength") != NONE)
      class = 0;

    else
      class = (havefood (3) ? 1000 : 0) +
                (Level > 12 ? 150 :
                 Str > 2000 ? 700 :
                 Str > 1600 ? Str - 1200 :
                   100);
  }

  /* A ring of searching */
  else if (stlmatch (inven[i].str, "searching"))
  { class = (havefood (0) ? 1000 : 0) + 250; }

  /* A ring of see invisible */
  else if (stlmatch (inven[i].str, "see invisible")) {
    /* A second ring of see invisible is useless */
    if (!itemis (i, INUSE) && wearing ("see invisible") != NONE)
      class = 0;

    /*
     * If we are beingstalked and we are wearing this ring, then
     * we should take it off and put it on to set the Rogue CANSEE
     * bit, which can be unset by a second ring of see invisible or
     * by a see invisible potion wearing off.				MLM
     */

    else if (itemis (i, INUSE) && beingstalked)
      class = 800;

    /*
     * When we put the ring on, keep its importance high for 20
     * turns, just in case the beast comes back to haunt us.		MLM
     */

    else
      class = (beingstalked || turns - putonseeinv < 20) ? 1999 :
                  ((havefood (0) && Level > 15 && Level < 26) ? 1000 : 0) + 300;
  }

  /* A ring of adornment */
  else if (stlmatch (inven[i].str, "adornment"))
  { class = 0; }

  /* A ring of aggravate monster */
  else if (stlmatch (inven[i].str, "aggravate monster"))
    { class = 0; }

  /* A ring of dexterity */
  else if (stlmatch (inven[i].str, "dexterity"))
    { if (magicplus > 0) class = (havefood (0) ? 1000 : 0) + 475; }

  /* A ring of increase damage */
  else if (stlmatch (inven[i].str, "increase damage"))
    { if (magicplus > 0) class = (havefood (0) ? 1000 : 0) + 500; }

  /* A ring of regeneration */
  else if (stlmatch (inven[i].str, "regeneration")) {
    /* Analysis indicates that rings of regenerate DO NOT hold back   */
    /* the character after any level. They each add one hit point per */
    /* turn of rest regardless of your level!			MLM   */

    class = 50*(Hpmax-Hp-Explev) + 500;
  }

  /* A ring of slow digestion */
  else if (stlmatch (inven[i].str, "slow digestion")) {
    /* A second ring of slow digestion is not too useful */
    if (havefood (0) && !itemis (i, INUSE) &&
        wearing ("slow digestion") != NONE)
      class = 1001;

    else {
      class =	havefood (3) ?	1100 :
                havefood (2) ?	1300 :
                havefood (1) ?	1500 :
                havefood (0) ?	1900 :
                  1999 ;
    }
  }

  /* A ring of teleportation */
  else if (stlmatch (inven[i].str, "telportation") ||
           stlmatch (inven[i].str, "teleportation"))
  { class = 0; }

  /* A ring of stealth */
  else if (stlmatch (inven[i].str, "stealth")) {
    /* A second ring of stealth is useless */
    if (!itemis (i, INUSE) && wearing ("stealth") != NONE)
      class = 0;

    else {
      class = (havefood (1) ? 1000 : 0) +
                  (Level > 17 ? 850 : Level > 12 ? 700 : 300);
    }
  }

  /* A ring of maintain armor */
  else if (stlmatch (inven[i].str, "maintain armor")) {
    int bestarm, nextarm, armdiff;

    /* No rust monsters yet or cursed armor */
    if (Level < 9 || cursedarmor) return (900);

    /* Past the rust monsters */
    else if (Level > 18) return (300);

    /* A second ring of maintain armor is useless */
    else if (!itemis (i, INUSE) && wearing ("maintain armor") != NONE)
      class = 0;

    else {
      bestarm = havearmor (1, NOPRINT, ANY);
      nextarm = havearmor (1, NOPRINT, RUSTPROOF);

      if (bestarm < 0)                       /* No armor to protect */
        return (700);

      else if (!willrust (bestarm))	     /* Armor wont rust anyway */
        return (0);

      else if (nextarm < 0)	             /* Naked is AC 10 */
        armdiff = 10 - armorclass (bestarm);

      else			             /* Get difference in classes */
        armdiff = armorclass (nextarm) -
                  armorclass (bestarm);

      class = (havefood (1) ? 1000 : 0) +
                  200 * armdiff;
    }
  }

  /* Not a known ring, forget it */
  else
    return (0);

  /* A magical plus is worth 100 */
  return (class + 100*magicplus);
}

/*
 * havebow: Return Kth best thrower. K should be in the range 1 to invcount.
 *          If K is greater than the number of weapons we have,
 *          then NONE is returned.
 */

int havebow (k, print)
{
  register int i, j, w, t, n=0;
  int bowind[MAXINV], bowval[MAXINV];

  for (i=0; i<invcount; ++i)
    if (inven[i].count && (w = bowclass (i)) > 0) {
      n++;

      for (j = n-1; j > 0 && w >= bowval[j-1]; j--) {
        swap (bowind[j], bowind[j-1]);
        swap (bowval[j], bowval[j-1]);
      }

      bowind[j] = i;
      bowval[j] = w;
    }

  if (print) {
    mvaddstr (1, 0, "Current Bow Rankings");

    for (i = 0; i<n; i++)
      mvprintw (i+3, 8, "%2d: %5d %s", i+1, bowval[i], itemstr (bowind[i]));
  }

  return ((k <= n) ? bowind[k-1] : NONE);
}

/*
 * bowclass: Given the index of a bow, return the bow class.
 *           This is the average damage done + 6/5 the plus to
 *           hit, multiplied by 10.
 */

int bowclass (i)
int i;
{
  int class, hitplus = 0, damplus = 0;

  if (inven[i].type == thrower &&
      stlmatch (inven[i].str, "short bow") &&
      havemult (missile, "arrow", 5) != NONE)
    class =  35;
  else
    return (0);

  /* Find the modifiers */
  if (inven[i].phit != UNKNOWN) {
    hitplus += inven[i].phit;

    if (inven[i].pdam != UNKNOWN)
      damplus = inven[i].pdam;
  }

  return (class + 12*hitplus + 10*damplus);
}

/*
 * havemissile: Return best missile. Dont consider arrows if we
 * are cheating.  Consider arrows first if we are wielding our bow.
 */

int havemissile ()
{
  register int i, fewest = 9999, obj = NONE;

  if (wielding (thrower)) {	/* Wielding bow, use arrows */
    for (i=0; i<invcount; ++i)
      if (inven[i].count > 0 && inven[i].count < fewest &&
          inven[i].type == missile && stlmatch(inven[i].str,"arrow"))
        { obj = i; fewest = inven[i].count; }
  }

  if (obj < 0) {		/* Not wielding bow or no arrows */
    for (i=0; i<invcount; ++i)
      if (inven[i].count > 0 &&
          inven[i].count < fewest &&
          !itemis (i, INUSE) &&
          (inven[i].type == missile ||
           stlmatch(inven[i].str,"spear") ||
           stlmatch(inven[i].str,"dagger") ||
           stlmatch(inven[i].str,"mace") && inven[i].phit <= 0 ||
           stlmatch(inven[i].str,"long sword") && inven[i].phit < 0))
        { obj = i; fewest = inven[i].count; }
  }

  if (obj != NONE)
    dwait (D_BATTLE, "Havemissile returns (%s", itemstr (obj));
  else
    dwait (D_BATTLE, "Havemissile fails");

  return (obj);
}

/*
 * havearrow: return the index of any arrow which has count 1.
 */

havearrow ()
{
  int arr;

  for (arr = 0; arr<invcount; arr++)
    if (inven[arr].type == missile &&
        inven[arr].count == 1 &&
        stlmatch(inven[arr].str,"arrow"))
      return (arr);

  return (NONE);
}

/*
 * hitbonus: Return the bonus to hit.
 */

int hitbonus (strength)
int strength;
{
  int bonus = 0;

  if (strength < 700) bonus = strength/100 - 7;

  else if (version_uses_new_strength()) {
    if (strength < 1700) bonus = 0;
    else if (strength < 2100) bonus = 1;
    else if (strength < 3100) bonus = 2;
    else bonus = 3;
  }

  else {
    if (strength < 1700) bonus = 0;
    else if (strength < 1851) bonus = 1;
    else if (strength < 1900) bonus = 2;
    else bonus = 3;
  }

  return (bonus);
}

/*
 * damagebonus: bonus = the damage bonus.
 */

int damagebonus (strength)
int strength;
{
  int bonus = 0;

  if (strength < 700) bonus = strength/100 - 7;

  else  if (version_uses_new_strength()) {
    if (strength < 1600) bonus = 0;
    else if (strength < 1800) bonus = 1;
    else if (strength < 1900) bonus = 2;
    else if (strength < 2100) bonus = 3;
    else if (strength < 2200) bonus = 4;
    else if (strength < 1600) bonus = 5;
    else bonus = 6;
  }

  else {
    if (strength < 1600) bonus = 0;
    else if (strength < 1800) bonus = 1;
    else if (strength < 1801) bonus = 2;
    else if (strength < 1876) bonus = 3;
    else if (strength < 1891) bonus = 4;
    else if (strength < 1900) bonus = 5;
    else bonus = 6;
  }

  return (bonus);
}

/*
 * setbonuses: Set global hit and damage pluses.
 */

void setbonuses ()
{
  /* Set global Hit bonus */
  gplushit = hitbonus (Str);

  if (leftring != NONE && stlmatch (inven[leftring].str, "dexterity") &&
      inven[leftring].phit != UNKNOWN)
    gplushit += inven[leftring].phit;

  if (rightring != NONE && stlmatch (inven[rightring].str, "dexterity") &&
      inven[rightring].phit != UNKNOWN)
    gplushit += inven[rightring].phit;

  /* Set global Damage Bonus */
  gplusdam = damagebonus (Str);

  if (leftring != NONE && stlmatch (inven[leftring].str, "add damage") &&
      inven[leftring].pdam != UNKNOWN)
    gplusdam += inven[leftring].pdam;

  if (rightring != NONE && stlmatch (inven[rightring].str, "add damage") &&
      inven[rightring].pdam != UNKNOWN)
    gplusdam += inven[rightring].pdam;

  /* Set bonuses for weapons */
  wplushit = gplushit;
  wplusdam = gplusdam;

  if (currentweapon != NONE) {
    if (inven[currentweapon].phit != UNKNOWN)
      wplushit += inven[currentweapon].phit;

    if (inven[currentweapon].pdam != UNKNOWN)
      wplusdam += inven[currentweapon].pdam;
  }
}
