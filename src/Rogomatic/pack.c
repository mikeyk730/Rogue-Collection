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
 * pack.c:
 *
 * This file contains functions which mess with Rog-O-Matics pack
 */

# include <curses.h>
# include <stdlib.h>
# include <string.h>
# include "types.h"
# include "globals.h"

static char *stuffmess [] = {
  "strange", "food", "potion", "scroll",
  "wand", "ring", "hitter", "thrower",
  "missile", "armor", "amulet", "gold",
  "none"
};

/*
 * itemstr: print the inventory message for a single item.
 */

char *itemstr (i)
register int i;
{
  static char ispace[128];
  register char *item = ispace;

  memset (ispace, '\0', 128);

  if (i < 0 || i >= MAXINV)
    { sprintf (item, "%d out of bounds", i); }
  else if (inven[i].count < 1)
    { sprintf (item, "%c)      nothing", LETTER(i)); }
  else {
    sprintf (item, "%c) %4d %d %s:", LETTER(i), worth(i),
             inven[i].count, stuffmess[(int)inven[i].type]);

    if (inven[i].phit != UNKNOWN && inven[i].pdam == UNKNOWN)
      sprintf (item, "%s (%d)", item, inven[i].phit);
    else if (inven[i].phit != UNKNOWN)
      sprintf (item, "%s (%d,%d)", item, inven[i].phit, inven[i].pdam);

    if (inven[i].charges != UNKNOWN)
      sprintf (item, "%s [%d]", item, inven[i].charges);

    sprintf (item, "%s %s%s%s%s%s%s%s%s%s",	  /* DR UTexas */
             item, inven[i].str,
             (itemis (i, KNOWN) ? "" : ", unk"),
             (used (inven[i].str) ? ", tried" : ""),
             (itemis (i, CURSED) ? ", cur" : ""),
             (itemis (i, UNCURSED) ? ", unc" : ""),
             (itemis (i, ENCHANTED) ? ", enc" : ""),
             (itemis (i, PROTECTED) ? ", pro" : ""),
             (itemis (i, WORTHLESS) ? ", useless" : ""),
             (!itemis (i, INUSE) ? "" :
              (inven[i].type == armor || inven[i].type == ring) ?
              ", on" : ", inhand"));
  }

  return (item);
}

/*
 * dumpinv: print the inventory. calls itemstr.
 */

void dumpinv (f)
register FILE *f;
{
  register int i;

  if (f == NULL)
    at (1,0);

  for (i=0; i<MAXINV; i++) {
    if (inven[i].count == 0)			/* No item here */
      ;
    else if (f != NULL)				/* Write to a file */
      { fprintf (f, "%s\n", itemstr (i)); }
    else {				/* Dump on the screen */
      printw ("%s\n", itemstr (i));
    }
  }
}

/*
 * removeinv: remove an item from the inventory.
 */

void removeinv (pos)
int pos;
{
  if (--(inven[pos].count) == 0) {
    clearpack  (pos);		/* Assure nothing at that spot  DR UT */

    forget (pos, (KNOWN | CURSED | ENCHANTED | PROTECTED | UNCURSED |
                  INUSE | WORTHLESS));

    rollpackup (pos);		/* Close up the hole */
  }

  countpack ();
  checkrange = 1;
}

/*
 * deleteinv: delete an item from the inventory. Note: this function
 * is used when we drop rather than throw or use, since bunches of
 * things can be dropped all at once.
 */

void deleteinv (pos)
int pos;
{

  if (--(inven[pos].count) == 0 || inven[pos].type == missile) {
    clearpack  (pos);		/* Assure nothing at that spot  DR UT */

    forget (pos, (KNOWN | CURSED | ENCHANTED | PROTECTED | UNCURSED |
                  INUSE | WORTHLESS));

    rollpackup (pos);		/* Close up the hole */
  }

  countpack ();
  checkrange = 1;
}

/*
 * clearpack: zero out slot in pack.  DR UTexas 01/05/84
 */

void clearpack (pos)
int pos;
{
  if (pos >= MAXINV) return;

  inven[pos].count = 0;
  inven[pos].str[0] = '\0';
  inven[pos].phit = UNKNOWN;
  inven[pos].pdam = UNKNOWN;
  inven[pos].charges = UNKNOWN;

  /* let's try remembering more stuff so we don't forget what is
     protected, cursed etc.

  forget (pos, (KNOWN | CURSED | ENCHANTED | PROTECTED | UNCURSED |
                INUSE | WORTHLESS));
  */

  forget (pos, ( INUSE ));
}

/*
 * rollpackup: We have deleted an item, move up the objects behind it in
 * the pack.
 */

void rollpackup (pos)
register int pos;
{
  register char *savebuf;
  register int i;

  if (!dynamic_inv_order()) return;

  if (pos < currentarmor) currentarmor--;
  else if (pos == currentarmor) currentarmor = NONE;

  if (pos < currentweapon) currentweapon--;
  else if (pos == currentweapon) currentweapon = NONE;

  if (pos < leftring) leftring--;
  else if (pos == leftring) leftring = NONE;

  if (pos < rightring) rightring--;
  else if (pos == rightring) rightring = NONE;

  savebuf = inven[pos].str;

  for (i=pos; i+1<invcount; i++)
    inven[i] = inven[i+1];

  inven[--invcount].str = savebuf;

  inven[invcount].count = 0; /* mark this slot as empty - NYM */
}

/*
 * rollpackdown: Open up a new spot in the pack, and move down the
 * objects behind that position.
 */

void rollpackdown (pos)
register int pos;
{
  register char *savebuf;
  register int i;

  if (!dynamic_inv_order()) {
    return;
  }

  savebuf = inven[invcount].str;

  for (i=invcount; i>pos; --i) {
    inven[i] = inven[i-1];

    if (i-1 == currentarmor)   currentarmor++;

    if (i-1 == currentweapon)  currentweapon++;

    if (i-1 == leftring)       leftring++;

    if (i-1 == rightring)      rightring++;
  }

  inven[pos].str = savebuf;

  if (++invcount > MAXINV) {
    usesynch = 0;
  }
}

/*
 * resetinv: send an inventory command. The actual work is done by
 * doresetinv.
 */

void resetinv()
{
  if (!replaying) {
    command (T_OTHER, "i");
  }
  else {
    /* if we are replaying, then the original game would have caused
     * doresetinv to be called via the command above, so just call it
     * directly.  If this isn't called the replay core dumps with a
     * segfault because the inventory structure is incorrect - NYM
     */
    doresetinv ();
  }
}

/*
 * doresetinv: reset the inventory.  DR UTexas 01/05/84
 */

void doresetinv ()
{
  int i;
  static char space[MAXINV][80];

  usesynch = 1;
  checkrange = 0;

  for(i=0; i<MAXINV; ++i) {
    inven[i].str = space[i];
    clearpack (i);
  }

  invcount = objcount = urocnt = 0;
  currentarmor = currentweapon = leftring = rightring = NONE;

  if (!dynamic_inv_order()) invcount = MAXINV;
}

/*
 * inventory: parse an item message.
 */

# define xtr(w,b,e,k) {what=(w);xbeg=mess+(b);xend=mend-(e);xknow|=(k);}

int inventory (msgstart, msgend)
char *msgstart, *msgend;
{
  register char *p, *q, *mess = msgstart, *mend = msgend;
  char objname[100];
  char dbname[NAMSIZ];
  char codename[NAMSIZ];
  int  n, ipos, xknow = 0, newitem = 0, inuse = 0, printed = 0, len = 0;
  int  plushit = UNKNOWN, plusdam = UNKNOWN, charges = UNKNOWN;
  stuff what;
  char *xbeg, *xend, *codenamebeg, *codenameend;

  xbeg = xend = codenamebeg = codenameend = "";
  dwait (D_PACK, "inv: message %s", mess);

  if (debug(D_MESSAGE)) {
    at (30,0);
    clrtoeol ();
    printw(DBG_FMT,mess);
    at (row, col);
    refresh ();
  }

  /* Rip surrounding garbage from the message */

  /* strange line end when reading identify scrolls, ignore it */
  if (*mend == '-')
    mend-=1;

  if (mess[1] == ')')
    { newitem = 1; ipos = DIGIT(*mess); mess += 3;}
  else
    { ipos = DIGIT(mend[-2]); mend -= 4; }


  if ((ipos < 0) || (ipos > MAXINV)) {
    len = msgend - msgstart;
    dwait (D_ERROR,
           "inv: ipos out of range, 0 - MAXINV(%d) ipos %d  invcount %d\nmsgs: %s\n  mess: %s\n",
           MAXINV, ipos, invcount, msgstart, mess);
    return(printed);
  }
  else {
    deletestuff (atrow, atcol);
    unsetrc (USELESS, atrow, atcol);
  }

  if (ISDIGIT(*mess))
    { n = atoi(mess); mess += 2+(n>9); }
  else {
    n = 1;

    if (*mess == 'a') mess++;   /* Eat the determiner A/An/The */

    if (*mess == 'n') mess++;

    if (*mess == 't') mess++;

    if (*mess == 'h') mess++;

    if (*mess == 'e') mess++;

    if (*mess == ' ') mess++;
  } /* Eat the space after the determiner */

  /* Read the plus to hit */
  if (*mess=='+' || *mess=='-') {
    plushit = atoi(mess++);

    while (ISDIGIT (*mess)) mess++;

    xknow = KNOWN;
  }

  /* Eat any comma separating two modifiers */
  if (*mess==',') mess++;

  /* Read the plus damage */
  if (*mess=='+' || *mess=='-') {
    plusdam = atoi(mess++);

    while (ISDIGIT (*mess)) mess++;

    xknow = KNOWN;
  }

  while (*mess==' ') mess++;		/* Eat any separating spaces */

  while (mend[-1]==' ') mend--;		/* Remove trailing blanks */

  while (mend[-1]=='.') mend--;		/* Remove trailing periods */

  /* Read any parenthesized strings at the end of the message */
  while (mend[-1]==')') {

    if (mend[-1] == ')')
      codenameend = mend-2;

    while (*--mend != '(') ;		/* on exit mend -> '(' */

    if (*mend == '(')
      codenamebeg = mend+1;

    if (stlmatch (mend,"(being worn)") )
      { currentarmor = ipos; inuse = INUSE; }
    else if (stlmatch (mend,"(weapon in hand)") )
      { currentweapon = ipos; inuse = INUSE; }
    else if (stlmatch (mend,"(on left hand)") )
      { leftring = ipos; inuse = INUSE; }
    else if (stlmatch (mend,"(on right hand)") )
      { rightring = ipos; inuse = INUSE; }

    while ((mend[-1]==' ') && (mend > mess)) mend--;
  }

  /* Read the charges on a wand (or armor class or ring bonus) */
  if (mend[-1] == ']') {
    while (*--mend != '[');		/* on exit mend -> '[' */

    if (mend[1] == '+')	charges = atoi(mend+2);
    else		charges = atoi(mend+1);

    xknow = KNOWN;
  }

  while (mend[-1] == ' ') mend--;

  /* Undo plurals by removing trailing 's' (but not for "blindne->ss<-") */
  if ((mend[-1]=='s') && (mend[-2] != 's')) mend--;

  /* Now find what we have picked up: */
  if (stlmatch(mend-4,"food")) {what=food; xknow=KNOWN;}
  else if (stlmatch(mess,"amulet")) {
      dosnapshot();
      xtr(amulet,0,0,KNOWN)
  }
  else if (stlmatch(mess,"potion of ")) xtr(potion,10,0,KNOWN)
  else if (stlmatch(mess,"potions of ")) xtr(potion,11,0,KNOWN)
  else if (stlmatch(mess,"scroll of ")) xtr(Scroll,10,0,KNOWN)
  else if (stlmatch(mess,"scrolls of ")) xtr(Scroll,11,0,KNOWN)
  else if (stlmatch(mess,"staff of ")) xtr(wand,9,0,KNOWN)
  else if (stlmatch(mess,"wand of ")) xtr(wand,8,0,KNOWN)
  else if (stlmatch(mess,"ring of "))  xtr(ring,8,0,KNOWN)
  else if (stlmatch(mess,"scrolls called ")) xtr(Scroll,15,0,KNOWN)
  else if (stlmatch(mess,"scroll called ")) xtr(Scroll,14,0,KNOWN)
  else if (stlmatch(mess,"scrolls titled '")) xtr(Scroll,16,1,0)
  else if (stlmatch(mess,"scroll titled '")) xtr(Scroll,15,1,0)
  else if (stlmatch(mess,"potions called ")) xtr(potion,15,0,KNOWN)
  else if (stlmatch(mess,"potion called ")) xtr(potion,14,0,KNOWN)
  else if (stlmatch(mess,"ring called ")) xtr(ring,12,0,KNOWN)
  else if (stlmatch(mess,"wand called ")) xtr(wand,12,0,KNOWN)
  else if (stlmatch(mess,"staff called ")) xtr(wand,13,0,KNOWN)
  else if (stlmatch(mess,"apricot")) xtr(food,0,0,KNOWN)
  else if (stlmatch(mess,"mango")) xtr(food,0,0,KNOWN)
  else if (stlmatch(mess,"slime-mold")) xtr(food,0,0,KNOWN)
  else if (stlmatch(mend-5,"arrow")) xtr(missile,0,0,0)
  else if (stlmatch(mend-8,"shuriken")) xtr(missile,0,0,0)
  else if (stlmatch(mend-6,"scroll")) xtr(Scroll,0,7,0)
  else if (stlmatch(mend-6,"potion")) xtr(potion,0,7,0)
  else if (stlmatch(mend-5,"staff")) xtr(wand,0,6,0)
  else if (stlmatch(mend-4,"wand"))  xtr(wand,0,5,0)
  else if (stlmatch(mend-4,"ring")) xtr(ring,0,5,0)
  else if (stlmatch(mend-4,"mail")) xtr(armor,0,0,0)
  else if (stlmatch(mend-5,"sword")) xtr(hitter,0,0,0)
  else if (stlmatch(mend-4,"mace")) xtr(hitter,0,0,0)
  else if (stlmatch(mend-6,"dagger")) xtr(missile,0,0,0)
  else if (stlmatch(mend-5,"spear")) xtr(missile,0,0,0)
  else if (stlmatch(mend-5,"armor")) xtr(armor,0,0,0)
  else if (stlmatch(mend-3,"arm")) xtr(armor,0,0,0)
  else if (stlmatch(mend-3,"bow")) xtr(thrower,0,0,0)
  else if (stlmatch(mend-5,"sling")) xtr(thrower,0,0,0)
  else if (stlmatch(mend-4,"dart")) xtr(missile,0,0,0)
  else if (stlmatch(mend-4,"rock")) xtr(missile,0,0,0)
  else if (stlmatch(mend-4,"bolt")) xtr(missile,0,0,0)
  else xtr(strange,0,0,0)

  /* Copy the name of the object into a string */
  memset (objname, '\0', 100);

  for (p = objname, q = xbeg; q < xend;  p++, q++) *p = *q;

  /* Ring bonus is printed differently in Rogue 5.3 */
  if (new_ring_format() && what == ring && charges != UNKNOWN)
    { plushit = charges; charges = UNKNOWN; }

  dwait (D_PACK, "inv %s '%s'",
         stuffmess[(int) what], objname);
  dwait (D_PACK, "inv    ht %d dm %d ch %d kn %d",
         plushit, plusdam, charges, xknow);

  /* make sure all unknown potion, Scroll, wand, rings
     are in dbase */
  if (!xknow && (what == potion || what == Scroll || what == wand || what == ring)) {
    addobj (objname, ipos, what);
  }

  /* If the name of the object matches something in the database, */
  /* slap the real name into the slot and mark it as known */
  if (!xknow && (what == potion || what == Scroll || what == wand || what == ring)) {
    memset (dbname, '\0', NAMSIZ);
    strncpy (dbname, findentry_getrealname (objname, what), NAMSIZ-1);

    if (strlen (dbname) > 0) {
      strcpy (objname, dbname);
      strcpy (pending_call_name, dbname);
      pending_call_letter = LETTER (ipos);
      xknow = KNOWN;

      if (newitem) {
        at (0,0);

        if (n == 1) printw ("a ");
        else printw ("%d ", n);

        printw ("%s%s of %s (%c)",
                what == potion ?    "potion" :
                what == Scroll ?  "scroll" :
                what == ring ?    "ring" :
                "wand",
                (n == 1) ? "" : "s",
                objname,
                LETTER(ipos));

        clrtoeol ();
        at (row, col);
        refresh ();
        printed++;
      }
    }
  }

  /* Once in a while a wand/staff becomes known by use.
     when we find one of these update that database entry */
  if ((xknow == KNOWN) &&
    (what == wand)) {

    /* Copy the codename of the object into a string */
    memset (codename, '\0', NAMSIZ);

    for (p = codename, q = codenamebeg; q <= codenameend;  p++, q++) *p = *q;

    if ((strlen (codename) > 2) &&
      (strlen (findentry_getrealname (codename, what)) == 0)) {
      infername (codename, objname, what);
    }
  }

  /* If new item, record the change */
  if (newitem && what == armor)
    newarmor = 1;
  else if (newitem && what == ring)
    newring = 1;
  else if (newitem && what == food)
    { newring = 1; lastfoodlevel = Level; }
  else if (newitem && (what == hitter || what == missile || what == wand))
    newweapon = 1;

  /* If the object is an old object, set its count, else allocate */
  /* a new object and roll the other objects down */

  if (n > 1 && ipos < invcount && inven[ipos].type == what &&
      n == inven[ipos].count+1 &&
      stlmatch(objname, inven[ipos].str) &&
      inven[ipos].phit == plushit &&
      inven[ipos].pdam == plusdam) {
    inven[ipos].count = n;
  }
  /* New item, in older Rogues, open up a spot in the pack */
  else {
    if (dynamic_inv_order()) {
      rollpackdown (ipos);
    }

    inven[ipos].type = what;
    inven[ipos].count = n;
    inven[ipos].phit = plushit;
    inven[ipos].traits = 0; /* mdk:bugfix clear traits for new items */

    if ((plushit != UNKNOWN) && (plushit > 0))
      remember (ipos, ENCHANTED | KNOWN);

    inven[ipos].pdam = plusdam;

    if ((plusdam != UNKNOWN) && (plusdam > 0))
      remember (ipos, ENCHANTED | KNOWN);

    inven[ipos].charges = charges;
    remember (ipos, inuse | xknow);

    if (!xknow) ++urocnt;
  }

  /* Forget enchanted status if item known.  DR UTexas 31 Jan 84 */
  /* ...why?...  no idea, let's put this back in and see what happens */
  /*
  if (itemis (ipos, KNOWN))
    {
      forget (ipos, ENCHANTED);
    }
  */

  /* Set the name of the object */
  if (inven[ipos].str != NULL) {
    if (strlen (objname) > 0)
      strcpy (inven[ipos].str, objname);
  }
  else if (!replaying) {
    dwait (D_ERROR, "inv: null inven[%d].str, invcount %d.",
           ipos, invcount);
  }

  /* Set cursed attribute for weapon and armor */
  if (cursedarmor && ipos == currentarmor) remember (ipos, CURSED);

  if (cursedweapon && ipos == currentweapon) remember (ipos, CURSED);

  if (debug(D_MESSAGE)) {
    at (30,0);
    clrtoeol ();
    printw(DBG_FMT2,mess);
    at (31,0);
    clrtoeol ();
    printw(DBG_FMT2,objname);
    at (row, col);
    refresh ();
  }

  /* Keep track of whether we are wielding a trap arrow */
  if (ipos == currentweapon) usingarrow = (what == missile);

  countpack ();

  /* If we picked up a useless thing, note that fact */
  if (newitem && on (USELESS))	remember (ipos, WORTHLESS);
  else if (newitem)		forget (ipos, WORTHLESS);

  checkrange = 1;

  return (printed);
}

/*
 * countpack: Count objects, missiles, and food in the pack.
 */

void countpack ()
{
  register int i, cnt;

  for (objcount=0, larder=0, ammo=0, i=0; i<invcount; i++) {
    if (! (cnt = inven[i].count))	; /* No object here */
    else if (inven[i].type == missile)	{ objcount++; ammo += cnt; }
    else if (inven[i].type == food)	{ objcount += cnt; larder += cnt; }
    else				{ objcount += cnt; }
  }
}
