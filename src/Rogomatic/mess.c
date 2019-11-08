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
 * mess.c:
 *
 * This file contains all of the functions which parse the message line.
 */

# include <curses.h>
# include <ctype.h>
# include <string.h>
# include <stdlib.h>
# include "types.h"
# include "globals.h"

/* Matching macros */
# define MATCH(p) smatch(mess,p,result)

/* Local data recording statistics */
static int monkilled[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static int totalkilled=0, timeshit=0, timesmissed=0, hits=0, misses=0;
static int sumgold=0, sumsqgold=0, numgold=0;

static mhit=0, mmiss=0, mtarget= NONE;

/* Other local data */
int identifying = 0;		/* Next message is from identify scroll */
static int justreadid = 0;	/* True if just read identify scroll */
static int gushed = 0;		/* True ==> water on the head msg recently */
static int echoit;		/* True ==> echo this message to the user */

/* Results from star matcher */
static char res1[NAMSIZ], res2[NAMSIZ], res3[NAMSIZ], res4[NAMSIZ], res5[NAMSIZ];
static char *result[] = { res1, res2, res3, res4, res5 };

/*
 * terpmes: called when a message from Rogue is on the top line,
 * this function parses the message and notes the information.
 * Note that the messages are all lower cased, to help with
 * compatability bewtween 3.6 and 5.2, since 5.2 capitalizes more
 * messages than does 3.6.  Trailing punctuation is also ignored.
 *
 * As of Rogue 5.3, multiple messages are broken into single
 * messages before being passed to parsemsg.  Periods separate
 * multiple messages.
 */

void terpmes ()
{
  char mess[128]; char topline[128];
  register char *m, *mend, *s, *t;

  s=&screen[0][0];
  memset (topline, '\0', 128);
  strncpy (topline, s, 127);
  s=topline;

  /* Set 't' to the tail of the message,
      skip backward until you find a letter, digit, or punctuation */
  t=topline+(MAXCOLS-1);

  while ((isspace(*t) || *t == '.' || *t == '-') && (t > topline)) {
    if (*t == '-' || *t == '.' || *t == '\0')
      *t = ' ';

    t--;
  }

  t++;  /* t -> beyond string */

  /*
   * Loop through each message, finding the beginning and end, and
   * copying it to mess, lower-casing it as we go. Then call parsemsg.
   */

  while (s<t) {				      /* While more chars in msg */
    memset (mess, '\0', 128);

    while (*s==' ' && s<t) s++;			/* Skip leading blanks */

    for (m = mess;				/* Copy text */
         s<t && (version < RV53A || *s != '.' || s[1] != ' ');
         s++)	{
      if (isprint(*s))
        *m = isupper (*s) ? tolower (*s) : *s;	  /* Lower case the char */

      m++;
    }

    s++;					/* Skip the period, if any */

    mend = m;

    /* :ANT: for debugging screen now has to be at least 31x80 */
    if debug(D_MESSAGE) {
      at (MAXROWS,0);
      printw (DBG_FMT,screen);
      at ((MAXROWS+1),0);
      printw (DBG_FMT,topline);
      at ((MAXROWS+2),0);
      clrtoeol ();
      printw (DBG_FMT,mess);
      refresh ();
    }

    /* :ANT: */

    if (mess != mend) parsemsg (mess, mend);	/* Parse it */

    /* :ANT: for debugging */
    if debug(D_MESSAGE) {
      at (MAXROWS,0);
      printw (DBG_FMT2,screen);
      at ((MAXROWS+1),0);
      printw (DBG_FMT2,topline);
      at ((MAXROWS+2),0);
      clrtoeol ();
      printw (DBG_FMT2,mess);
      refresh ();
    }

    /* :ANT: */
  }
}

/*
 * parsemsg: Parse a single message, and if necessary set variables
 * or call functions.
 */

void parsemsg (mess, mend)
register char *mess, *mend;
{
  int unknown = 0;

  echoit = 1;

  /*----------------Take action based on type of message-------------*/

  /* :ANT: let's tag this as a BEARTRP for now */
  if (MATCH("* sparks dance across your armor*"))
    nametrap (BEARTRP,HERE);

  /* :ANT: */

  /* nymph stole an item, pack is out of whack */
  else if (MATCH("she stole *")) {
    usesynch=0;
  }

  /* Message indicates we picked up a new item */
  else if (*(mend-1)==')' && *(mend-3)=='(') {
    inventory (mess, mend);
    identifying = justreadid = 0;
    usesynch=0;
  }
  /* Message describes an old item already in our pack */
  else if (mess[1]==')') {
    echoit = identifying;
    identifying = justreadid = 0;
    inventory (mess, mend);
  }
  /* A random message, switch of first char to save some time... */
  else switch (mess[0]) {
      case 'a':

        if (MATCH("a secret door*")) echoit=0;
        else if (MATCH("as you read the scroll, it vanishes*")) echoit=0;
        else if (MATCH("a cloak of darkness falls around you*"))
          { infer ("blindness", potion); blinded=1; }
        else if (MATCH("a teleport trap*")) nametrap (TELTRAP,NEAR);
        else if (MATCH("a trapdoor*")) nametrap (TRAPDOR,NEAR);
        else if (MATCH("an arrow shoots *"))
          { arrowshot=1; nametrap(ARROW,HERE); }
        else if (MATCH("an arrow trap*")) nametrap (ARROW,NEAR);
        else if (MATCH("a beartrap*")) nametrap (BEARTRP,NEAR);
        else if (MATCH("a strange white mist *")) nametrap (GASTRAP,HERE);
        else if (MATCH("a sleeping gas trap*")) nametrap (GASTRAP,NEAR);
        else if (MATCH("a small dart *")) nametrap (DARTRAP,HERE);
        else if (MATCH("a dart trap*")) nametrap (DARTRAP,NEAR);
        else if (MATCH("a poison dart trap*")) nametrap (DARTRAP,NEAR);
        else if (MATCH("a rust trap*")) nametrap (WATERAP,NEAR);
        else if (MATCH("a gush of water hits you on the head*")) gushed++;
        else if (MATCH("a sting has weakened you*")) ;
        else if (MATCH("a bite has weakened you*")) ;

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("a mysterious trap*")) nametrap (BEARTRP,NEAR);
        else if (MATCH("a spike shoots past your ear!*")) nametrap (BEARTRP,HERE);
        else if (MATCH("a * light flashes in your eyes*")) nametrap (BEARTRP,HERE);

        /* :ANT: */

        else if (MATCH("a new monster is nearby*")) infer ("create monster", Scroll);
        else if (MATCH("a staff of * [*](*)*")) infer(res1, wand);
        else if (MATCH("a staff of * [*]*")) infer(res1, wand);
        else if (MATCH("a wand of * [*](*)*")) infer(res1, wand);
        else if (MATCH("a wand of * [*]*")) infer(res1, wand);
        else if (MATCH("a ring of *(*)*")) infer(res1, ring);
        else if (MATCH("a ring of *")) infer(res1, ring);
        else if (MATCH("a wand of *(*)*")) infer(res1, wand);
        else if (MATCH("a wand of *")) infer(res1, wand);
        else if (MATCH("a staff of *(*)*")) infer(res1, wand);
        else if (MATCH("a staff of *")) infer(res1, wand);
        else if (MATCH("a scroll of *")) infer (res1, Scroll);
        else if (MATCH("a potion of *(*)*")) infer(res1, potion);
        else if (MATCH("a potion of *")) infer(res1, potion);
        else if (MATCH("a +*")) ;
        else if (MATCH("an +*")) ;
        else if (MATCH("a -*")) ;
        else if (MATCH("an -*")) ;
        else unknown++;

        break;

      case 'b':

        if (MATCH("bolt bounces*")) infer ("lightning", wand);
        else if (MATCH("bolt hits*")) infer ("lightning", wand);
        else if (MATCH("bolt misses*")) infer ("lightning", wand);
        else if (MATCH("bummer, this food tastes awful*")) ;
        else if (MATCH("bummer!  you've hit the ground*")) floating=0;
        else if (MATCH("bite has no effect*")) ;
        else unknown++;

        break;

      case 'c':

        if (MATCH("call it*")) { echoit=0; }
        else if (MATCH("call what*")) { echoit=0; }
        else if (MATCH("called *")) { echoit=0; } /* mdk: call() shows existing name in my versions of Rogue */
        else unknown++;

        break;

      case 'd':

        if (MATCH("defeated the *")) { echoit=0; killed(res1); }
        else if (MATCH("defeated it*")) { echoit=0; killed("it"); }
        else if (MATCH("defeated *")) { echoit=0; killed(res1); }
        else if (MATCH("drop what*")) echoit=0;
        else if (MATCH("destroyed *"))
          { darkturns = 0; darkdir = NONE; targetmonster = 0; echoit=0; }
        else if (MATCH("dropped a scroll * scare monster")) {
          set (STUFF | SCAREM);
          diddrop = 1;
        }
        else if (MATCH("dropped *")) {
          set (STUFF | USELESS);
          diddrop = 1;
        }
        else unknown++;

        break;

      case 'e':

        if (MATCH("eat what*")) echoit=0;
        else if (MATCH("everything looks so boring now*")) cosmic=0;
        else unknown++;

        break;

      case 'f':

        if (MATCH("flame *")) ;
        else if (MATCH("far out!  everything is all cosmic again*")) blinded=0;
        else unknown++;

        break;

      case 'g':

        if (MATCH("getting hungry*")) echoit=0;
        else if (MATCH("getting the munchies*")) echoit=0;
        else unknown++;

        break;

      case 'h':

        if (MATCH("hey, this tastes great*")) infer ("restore strength", potion);
        else if (MATCH("huh? what? who?*")) ;
        else if (MATCH("heavy!  that's a nasty critter!*")) ;
        else unknown++;

        break;

      case 'i':

        if (MATCH("it hit*")) { washit ("it"); echoit=0; }
        else if (MATCH("it misses*"))  { wasmissed ("it"); echoit=0; }
        else if (MATCH("it appears confused*")) ;
        else if (MATCH("ice *")) ;
        else if (MATCH("identify what*")) echoit=0;
        else if (MATCH("illegal command*")) echoit=0;
        else if (MATCH("i see no way*"))
          { unset (STAIRS); findstairs (atrow, atcol); }
        else if (MATCH("it appears to be cursed*")) curseditem ();
        else if (MATCH("it make*")) ;
        else unknown++;

        break;

      case 'j':
      case 'k':
        unknown++;
        break;

      case 'l':

        if (MATCH("left or*")) echoit=0;
        else unknown++;

        break;

      case 'm':

        if (MATCH("missile vanishes*")) infer ("magic missile", wand);
        else if (MATCH("missle vanishes*")) infer ("magic missile", wand);
        else if (MATCH("my, that was a yummy *")) ;
        else if (MATCH("moved onto *")) set (STUFF);

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("multi-colored lines swirl around you, then fade*")) nametrap (BEARTRP,HERE);

        /* :ANT: */

        else unknown++;

        break;

      case 'n':

        if (MATCH("nothing happens*")) {
          remember (lastwand, WORTHLESS);
        }
        else if (MATCH("no more *")) ;
        else if (MATCH("nothing appropriate*")) sendnow ("%c;",ESC);
        else if (MATCH("no room*")) ;
        else if (MATCH("not wearing armor*")) ;
        else unknown++;

        break;

      case 'o':

        if (MATCH("oh no! an arrow shot *"))
          { arrowshot=1; nametrap(ARROW,HERE); }
        else if (MATCH("oh, now this scroll has a map *"))
          { infer ("magic mapping", Scroll); didreadmap = Level; }
        else if (MATCH("oh, bummer!  everything is dark!  help!*"))
          { infer ("blindness", potion); blinded=1; }
        else if (MATCH("oh, wow!  everything seems so cosmic!*"))
          { infer ("hallucination", potion); cosmic=1; }
        else if (MATCH("oh, wow!  you're floating in the air!*"))
          { infer ("levitation", potion); floating=1; }
        else if (MATCH("oh, wow, that tasted good*")) ;
        else unknown++;

        break;

      case 'p':

        if (MATCH("please spec*")) echoit=0;
        else if (MATCH("put on what*")) echoit=0;
        else unknown++;

        break;

      case 'q':

        if (MATCH("quaff what*")) echoit=0;
        else unknown++;

        break;

      case 'r':

        if (MATCH("range is 'a' to '*'*")) {
          echoit=0;

          if (*res1-'a'+1 != invcount) {
            dwait (D_INFORM, "Range check failed...");
            usesynch = 0;
          }
        }
        else if (MATCH("read what*")) echoit=0;
        else if (MATCH("rogue version *")) echoit=0;
        else unknown++;

        break;

      case 's':

        if (MATCH("she stole *")) {
          usesynch=0;
        }
        else if (MATCH("sting has no effect*")) ;
        else unknown++;

        break;

      case 't':

        if (MATCH("throw what*")) echoit=0;
        else if (MATCH("the screen looks fine*"));
        else if (MATCH("the * bounces*")) ;
        else if (MATCH("the bolt *")) ;
        else if (MATCH("the flame *")) ;
        else if (MATCH("the ice hits*")) ;
        else if (MATCH("the ice misses*")) ;
        else if (MATCH("the ice whizzes by you*")) wasmissed("ice monster");
        else if (MATCH("the * whizzes past*")) echoit = 0;
        else if (MATCH("the * hits it*")) {echoit=0; mshit ("it");}
        else if (MATCH("the * misses it*")) {echoit=0; msmiss ("it");}
        else if (MATCH("the * hits the *")) {echoit=0; mshit (res2);}
        else if (MATCH("the * misses the *")) {echoit=0; msmiss (res2);}
        else if (MATCH("the * hit*")) { washit (res1); gushed=0; echoit=0; }
        else if (MATCH("the * misses*")) { wasmissed (res1); echoit=0; }
        else if (MATCH("the * appears confused*")) ;
        else if (MATCH("the rust vanishes instantly*"))
          { if (gushed) { gushed = 0; nametrap (WATERAP, HERE); } }
        else if (MATCH("the room is lit*")) { setnewgoal (); infer ("light", wand); }
        else if (MATCH("the corridor glows*")) { infer ("light", wand); }
        else if (MATCH("the * has confused you*")) confused = 1;
        else if (MATCH("this scroll is an identify scroll scroll*"))
          { readident ("identify scroll"); }
        else if (MATCH("this scroll is an * scroll*")) {
          if (stlmatch (res1, "identify")) {
            readident (res1);
          }
        }
        else if (MATCH("that's not a valid item*")) ;
        else if (MATCH("the veil of darkness lifts*")) blinded=0;
        else if (MATCH("the scroll turns to dust*"))
          { deletestuff (atrow, atcol); unset(SCAREM | STUFF); droppedscare--; }
        else if (MATCH("this potion tastes * dull*")) infer ("thirst quenching", potion);
        else if (MATCH("this potion tastes pretty*")) infer ("thirst quenching", potion);
        else if (MATCH("this potion tastes like * juice*"))
          { infer ("see invisible", potion); if (version == RV36A) sendnow ("%c", ESC); }
        else if (MATCH("this scroll seems to be blank*")) infer ("blank paper", Scroll); /* mdk:bugfix: missing argument */
        else if (MATCH("the * bounces*")) ;
        else if (MATCH("the * vanishes as it hits the ground*"))
          { darkturns = 0; darkdir = NONE; targetmonster = 0; echoit=0; }
        else if (MATCH("there is something there already*")) {
          set(STUFF);
          usesynch=0;
        }
        else if (MATCH("there is something here*")) {
          set(STUFF);
          usesynch=0;
        }
        else if (MATCH("the munchies are interfering*")) ;
        else if (MATCH("the monsters around you freeze*")) holdmonsters ();
        else if (MATCH("the monster freezes*")) holdmonsters ();
        else if (MATCH("that's inedible*")) ;

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("time now seems to be going slower*")) nametrap (BEARTRP,HERE);
        else if (MATCH("the light in here suddenly seems*")) nametrap (BEARTRP,HERE);

        /* :ANT: */

        else unknown++;

        break;

      case 'u':
      case 'v':

        if (MATCH("version *")) echoit=0;
        else unknown++;

        break;

      case 'w':

        if (MATCH("what do you want*")) echoit=0;
        else if (MATCH("wield wha*")) {
          echoit=0;
          remember (lastdrop, UNCURSED);
          cursedweapon=0;
        }
        else if (MATCH("wielding a*")) ;
        else if (MATCH("wear what*")) echoit=0;
        else if (MATCH("what monster*")) echoit=0;
        else if (MATCH("wait, what's going*")) {infer("confusion", potion); confused=1;}
        else if (MATCH("wait*that's a *")) ;
        else if (MATCH("what a*feeling*")) { infer("confusion", potion); confused=1; }
        else if (MATCH("what a*piece of paper*")) infer ("blank paper", Scroll);
        else if (MATCH("what a bizarre schtick*")); //todo:mdk infer nothing
        else if (MATCH("welcome to level *")) ;
        else if (MATCH("was wearing*")) ;
        else if (MATCH("what bulging muscles*")) infer ("gain strength", potion);
        else if (MATCH("wearing *")) ;
        else unknown++;

        break;

      case 'x':
        unknown++;
        break;

      case 'y':

        if (MATCH("you hit*")) { echoit=0; didhit (); }
        else if (MATCH("you miss*")) { echoit=0; didmiss (); }
        else if (MATCH("you are starting to feel weak*")) echoit=0;
        else if (MATCH("you are weak from hunger*")) {echoit=0; eat();}
        else if (MATCH("you are being held*")) beingheld=30;
        else if (MATCH("you can move again*")) echoit=0;
        else if (MATCH("you are still stuck *")) nametrap (BEARTRP,HERE);
        else if (MATCH("you can't move*")) echoit=0;
        else if (MATCH("you are hit by the*")) echoit = 0;
        else if (MATCH("you can't carry anything else*"))
          { echoit=0; set (STUFF); maxobj=objcount; }
        else if (MATCH("you can*")) curseditem ();
        else if (MATCH("you begin to feel better*")) infer ("healing", potion);
        else if (MATCH("you begin to feel much better*")) infer("extra healing", potion);
        else if (MATCH("you begin to sense the presence of monsters*"))
          { infer("monster detection", potion); }
        else if (MATCH("you feel a strange sense of loss*")) infer("hold monster", Scroll);
        else if (MATCH("you feel a wrenching sensation in your gut*")) ;
        else if (MATCH("you feel stronger, now*")) infer ("gain strength", potion);
        else if (MATCH("you feel very sick now*")) infer ("poison", potion);
        else if (MATCH("you feel momentarily sick*")) infer ("poison", potion);
        else if (MATCH("you suddenly feel much more skillful*"))
          { infer("raise level", potion); }
        else if (MATCH("your nose tingles*")) infer ("food detection", Scroll);
        else if (MATCH("you hear a growling*")) infer("food detection", Scroll);
        else if (MATCH("you start to float in the air*"))
          { infer ("levitation", potion); floating=1; }
        else if (MATCH("you're floating off the ground!*")) floating=1;
        else if (MATCH("you float gently to the ground*")) floating=0;
        else if (MATCH("you feel yourself moving much faster*"))
          { infer ("haste self", potion); hasted = 1; }
        else if (MATCH("you feel yourself slowing down*"))
          { hasted = 0; doublehasted = 0; }
        else if (MATCH("you faint from exhaustion*"))
          { if (has_double_haste_bug()) doublehasted = 1; else hasted = 0; }
        else if (MATCH("you feel less confused now*")) confused = 0;
        else if (MATCH("you feel less trip*")) confused = 0;
        else if (MATCH("your * vanishes as it hits the ground*"))
          { darkturns = 0; darkdir = NONE; echoit=0; }
        else if (MATCH("your hands begin to glow *"))
          { infer ("monster confusion", Scroll); redhands = 1; }
        else if (MATCH("your hands stop glowing *")) redhands = 0;
        else if (MATCH("your mace gives off a flash*")); //todo:mdk handle vorp

        else if (MATCH("you feel as if somebody is watching over you*") ||
                 MATCH("you feel in touch with the universal onenes*")) {
          infer ("remove curse", Scroll);
          forget (currentarmor, CURSED);
          remember (currentarmor, UNCURSED);
          cursedarmor=0;
          forget (currentweapon, CURSED);
          remember (currentweapon, UNCURSED);
          cursedweapon=0;
          newarmor = newweapon = 1;
        }

        else if (MATCH("your armor weakens*")) {
          inven[currentarmor].phit--;

          if (gushed) { gushed=0; nametrap (WATERAP,HERE); }
        }

        else if (MATCH("your scalp itches")) infer ("protect armor", Scroll);
        else if (MATCH("your armor is covered by a shimmering * shield*")) {
          infer ("protect armor", Scroll);
          forget (currentarmor, CURSED);
          remember (currentarmor, UNCURSED);
          cursedarmor = 0;
          protected++;
          remember (currentarmor, PROTECTED);
        }

        else if (MATCH("your arms tingle")) infer ("enchant armor", Scroll);
        else if (MATCH("your armor glows * for a moment*")) {
          infer ("enchant armor", Scroll);
          cursedarmor = 0;
          newarmor = 1;
          inven[currentarmor].phit++;

          if (itemis(currentarmor, UNCURSED))
            remember (currentarmor, ENCHANTED);

          forget (currentarmor, CURSED);
          remember (currentarmor, UNCURSED);
        }
        else if (MATCH("your hands tingle")) infer ("enchant weapon", Scroll);
        else if (MATCH("your * glows * for a moment*")) {
          infer ("enchant weapon", Scroll);
          cursedweapon = 0;
          newweapon = 1;

          if (itemis (currentweapon, UNCURSED))
            remember (currentweapon, ENCHANTED);

          forget (currentweapon, CURSED);
          remember (currentweapon, UNCURSED);
        }
        else if (MATCH("you hear a high pitched humming noise*"))
          { infer ("aggravate monsters", Scroll); wakemonster (9); aggravated = 1; }
        else if (MATCH("you hear maniacal laughter*")) infer ("scare monster", Scroll);
        else if (MATCH("you hear a faint cry*")) infer ("create monster", Scroll);
        else if (MATCH("you fall asleep*")) infer ("sleep", Scroll);
        else if (MATCH("you have been granted the boon of genocide*"))
          { infer ("genocide", Scroll); echoit=0; rampage (); }
        else if (MATCH("you have a tingling feeling*")) infer ("drain life", wand);
        else if (MATCH("you are too weak to use it*")) infer ("drain life", wand);
        else if (MATCH("you begin to feel greedy*")) infer ("gold detection", potion);
        else if (MATCH("you feel a pull downward*")) infer ("gold detection", potion);
        else if (MATCH("you begin to feel a pull downward*"))
          { infer ("gold detection", potion); }
        else if (MATCH("you are caught *")) nametrap (BEARTRP,HERE);
        else if (MATCH("your purse feels lighter*")) ;
        else if (MATCH("you suddenly feel weaker*")) ;
        else if (MATCH("you must identify something*")) ;
        else if (MATCH("you have a * feeling for a moment, then it passes*")) infer ("monster detection", potion); //todo:mdk
        else if (MATCH("you have a * feeling for a moment*")) infer ("monster detection", potion);
        else if (MATCH("you daydream of * for a moment, then it passes*")) infer ("magic detection", potion);
        else if (MATCH("you feel deeply moved*")) infer ("teleportation", Scroll);

        else if (MATCH("you are transfixed*")) ;
        else if (MATCH("you are frozen*")) washit ("ice monster");
        else if (MATCH("you faint*")) {echoit=0; if (eat_after_fainting()) eat();}
        else if (MATCH("you freak out*")) echoit = 0;
        else if (MATCH("you fell into a trap!*")) ;
        else if (MATCH("you are damaged by the fall*"));
        else if (MATCH("yum*")) echoit=0;
        else if (MATCH("yuk*")) echoit=0;
        else if (MATCH("you sense the presence of magic*")) { infer ("magic detection", potion); echoit=0; }

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("you are suddenly in a parallel dimension*")) nametrap (BEARTRP, HERE);
        else if (MATCH("you feel a sting in the side of your neck*")) nametrap (BEARTRP, HERE);
        else if (MATCH("you feel time speed up suddenly*")) nametrap (BEARTRP, HERE);
        else if (MATCH("you suddenly feel very thirsty*")) nametrap (BEARTRP, HERE);
        else if (MATCH("yo* pack turns *")) nametrap (BEARTRP, HERE);

        /* :ANT: */

        /* :ANT: logic error indicator */
        else if (MATCH("you are already wearing some*"))
          dwait (D_ERROR, "Logic error, we should know this already. '%s'", mess);

        /* :ANT: */

        else unknown++;

        break;

      case 'z':

        if (MATCH("zap with what*")) echoit=0;
        else unknown++;

        break;

      default:

        if (MATCH( "* gold pieces*")) { echoit=0; countgold (res1); }
        else if (MATCH("(mctesq was here)*")) echoit=0;
        else if (MATCH("'*'*: *")) { echoit=0; mapcharacter (*res1, res3); }
        else if (*mess == '+' || *mess == '-' || ISDIGIT (*mess)) echoit=0;
        else if (MATCH("'*' is not a valid item*")) echoit=0;
        else unknown++;

        break;
    }

  /* Log unknown or troublesome messages */
  if ((morecount > 200) && (morecount < 4000)) {
    dwait(D_WARNING, "More Loop ->%s<-.", mess);
  }
  else if (morecount >= 4000) {
    dwait(D_FATAL, "More Loop Exit ->%s<-.", mess);
  }
  else if (unknown)
    dwait (D_WARNING, "Unknown message '%s'", mess);

  /* Send it to dwait; if dwait doesnt print it (and echo is on) echo it */
  if (echoit & !dwait (D_MESSAGE, mess))
    saynow (mess);
}

/*
 * smatch: Given a data string and a pattern containing one or
 * more embedded stars (*) (which match any number of characters)
 * return true if the match succeeds, and set res[i] to the
 * characters matched by the 'i'th *.
 */

int smatch (dat, pat, res)
register char *dat, *pat, **res;
{
  register char *star = 0, *starend, *resp;
  int nres = 0;

  while (1) {
    if (*pat == '*') {
      star = ++pat; 			     /* Pattern after * */
      starend = dat; 			     /* Data after * match */
      resp = res[nres++]; 		     /* Result string */
      *resp = '\0'; 			     /* Initially null */
    }
    else if (*dat == *pat) {	     /* Characters match */
      if (*pat == '\0') 		     /* Pattern matches */
        return (1);

      pat++; 				     /* Try next position */
      dat++;
    }
    else {
      if (*dat == '\0') 		     /* Pattern fails - no more */
        return (0); 			     /* data */

      if (star == 0) 			     /* Pattern fails - no * to */
        return (0); 			     /* adjust */

      pat = star; 			     /* Restart pattern after * */
      *resp++ = *starend; 		     /* Copy character to result */
      *resp = '\0'; 			     /* null terminate */
      dat = ++starend; 			     /* Rescan after copied char */
    }
  }
}

/*
 * readident: we have read an identify scroll.
 */

void readident (name)
char *name;
{
  int obj; char id = '*';	/* Default is "* for list" */
  stuff item_type = none;
  char lookup_name[NAMSIZ];

  if (!replaying && dynamic_inv_order() &&
      (nextid < LETTER (0) || nextid > LETTER (invcount))) {
    dwait (D_FATAL, "Readident: nextid %d, afterid %d, invcount %d.",
           nextid, afterid, invcount);
  }

  infer (name, Scroll);		/* Record what kind of scroll this is */

  at (0,0);
  clrtoeol ();
  memset (screen,' ', MAXCOLS);
  at (row, col);
  refresh ();

  if (has_universal_identify_scroll()) {	/* Rogue 3.6, Rogue 5.2 */
    deleteinv (OBJECT (afterid));	/* Assume object gone */
    sendnow (" %c", nextid);		/* Identify it */

    if (!replaying)   /* FIXME: without removing the rogo_send call during
                       * replay, replay will core dump - NYM
                       */
    {
      rogo_send ("I%c", afterid);		/* Generate a message about it */
    }

    knowident = identifying = 1;	/* Set variables */
  }
  else {			/* Rogue 5.3 */
    if (streq (name, "identify ring, wand or staff")) {
      if ((obj = unknown (ring)) != NONE) {
        id = LETTER (obj);
        item_type = ring;
      }
      else if ((obj = unknown (wand)) != NONE) {
        id = LETTER (obj);
        item_type = wand;
      }
      else if ((obj = have (ring)) != NONE) {
        id = LETTER (obj);
        item_type = ring;
      }
      else if ((obj = have (wand)) != NONE) {
        id = LETTER (obj);
        item_type = wand;
      }
    }
    else if (streq (name, "identify potion")) {
      if ((obj = unknown (potion)) != NONE || (obj = have (potion)) != NONE) {
        id = LETTER (obj);
        item_type = potion;
      }
    }
    else if (streq (name, "identify armor")) {
      if ((obj = unknown (armor)) != NONE || (obj = have (armor)) != NONE)
        id = LETTER (obj);
    }
    else if (streq (name, "identify weapon")) {
      if ((obj = unknown (hitter)) != NONE ||
          (obj = unknown (thrower)) != NONE ||
          (obj = unknown (missile)) != NONE ||
          (obj = have (hitter)) != NONE ||
          (obj = have (thrower)) != NONE ||
          (obj = have (missile)) != NONE)
        id = LETTER (obj);
    }
    else if (streq (name, "identify scroll") || streq (name, "identify")) {
      if ((obj = unknown (Scroll)) != NONE || (obj = have (Scroll)) != NONE) {
        id = LETTER (obj);
        item_type = Scroll;
      }
    }

    if ((id != '*') &&
        (item_type == ring || item_type == wand || item_type == potion || item_type == Scroll)) {
      memset (lastname, '\0', NAMSIZ);
      memset (lookup_name, '\0', NAMSIZ);
      strncpy (lookup_name, inven[obj].str, NAMSIZ-1);
      strncpy (lastname, findentry_getfakename (lookup_name, item_type), NAMSIZ-1);
    }

    waitfor ("not a valid item");
    /* sendnow (" %c;", id); */		/* Pick an object to identify */
    sendnow (" %c", id); /* mdk: removed extra semicolon */
    if (id == '*')
      memset (lastname, '\0', NAMSIZ);
    usesynch = 0; justreadid=1;		/* Must reset inventory */
  }

  newring = newweapon = 1; afterid = nextid = '\0';
}

/*
 * rampage: read a scroll of genocide.
 */

void rampage ()
{
  char monc;

  /* Check the next monster in the list, we may not fear him */
  while (monc = *genocide) {
    /* Do not waste genocide on stalkers if we have the right ring */
    if ((streq (monname (monc), "invisible stalker") ||
         streq (monname (monc), "phantom")) &&
        havenamed (ring, "see invisible") != NONE)
      { genocide++; }

    /* Do not waste genocide on rusties if we have the right ring */
    else if ((streq (monname (monc), "rust monster") ||
              streq (monname (monc), "aquator")) &&
             havenamed (ring, "maintain armor") != NONE)
      { genocide++; }

    /* No fancy magic for this monster, use the genocide scroll */
    else break;
  }

  /* If we found a monster, send his character, else send ESC */
  if (monc) {
    saynow ("About to rampage against %s", monname (monc));
    sendnow (" %c;", monc);	/* Send the monster */

    /* Add to the list of 'gone' monsters */
    sprintf (genocided, "%s%c", genocided, monc);
    genocide++;
  }
  else {
    dwait (D_ERROR, "Out of monsters to genocide!");
    sendnow (" %c;", ESC);	/* Cancel the command */
  }
}

/*
 * curseditem: the last object we tried to drop (unwield, etc.)  was cursed.
 *
 * Note that cursed rings are not a problem since we only put on
 * Good rings we have identified, so don't bother marking rings.
 */

void curseditem ()
{
  usesynch = 0;    /* Force a reset inventory */

  clearsendqueue();
  sendnow ("%c", ESC);
  g_expect_extra_bytes = 1;

  /* lastdrop is index of last item we tried to use which could be cursed */
  if (lastdrop != NONE && lastdrop < invcount) {
    remember (lastdrop, CURSED);

    /* Is our armor cursed? */
    if (inven[lastdrop].type == armor)
      { currentarmor = lastdrop; cursedarmor = 1; return; }

    /* Is it our weapon (may be wielding a hitter or a bogus magic arrow)? */
    else if (inven[lastdrop].type==hitter || inven[lastdrop].type==missile)
      { currentweapon = lastdrop; cursedweapon = 1; return; }
  }

  /* Don't know what was cursed, so assume the worst */
  cursedarmor=1;
  cursedweapon=1;

}

/*
 * First copy the title of the last scroll into the appropriate slot,
 * then find the real name of the object by looking through the data
 * base, and then zap that name into all of the same objects
 */

void infer (objname, item_type)
char *objname;
stuff item_type;
{
  register int i;

  if (*lastname && *objname && !stlmatch (objname, lastname)) {
    infername (lastname, objname, item_type);
    for (i=0; i<MAXINV; i++) {
      if ((inven[i].count > 0) &&
           streq (inven[i].str, lastname) && inven[i].type == item_type) {
        memset (inven[i].str, '\0', NAMSIZ);
        strncpy (inven[i].str, objname, NAMSIZ-1);
        remember (i, KNOWN);
      }
    }
  }
}

/*
 * Killed: called whenever we defeat a monster.
 */

void killed (monster)
register char *monster;
{
  register int m = 0, mh = 0;

  /* Find out what we really killed */
  if (!cosmic && !blinded && targetmonster>0 && streq (monster, "it"))
    { monster = monname (targetmonster); }

  if ((mh = getmonhist (monster, 0)) != NONE)
    { monster = monhist[mh].m_name; m = monsternum (monster); }

  /* Tell the user what we killed */
  dwait (D_BATTLE | D_MONSTER, "Killed '%s'", monster);

  /* If cheating against Rogue 3.6, check out our arrow */
  if (version_has_arrow_bug() && cheat) {
    if (usingarrow && hitstokill > 1 && !beingstalked && goodarrow < 20) {
      saynow ("Oops, bad arrow...");
      newweapon = badarrow = 1; remember (currentweapon, WORTHLESS);
    }
    else if (usingarrow) goodarrow++;
  }

  /* Echo the number arrows we pumped into him */
  if (mh >=0 && mhit+mmiss > 0 && mtarget == mh)
    dwait (D_BATTLE | D_MONSTER, "%d out of %d missiles hit the %s",
           mhit, mhit+mmiss, monster);

  /* If we killed it by hacking, add the result to long term memory */
  if (hitstokill > 0 && mh != NONE)
    addstat (&monhist[mh].htokill, hitstokill);

  /* If we killed it with arrows, add that fact to long term memory */
  if (mhit > 0 && mh != NONE)
    addstat (&monhist[mh].atokill, mhit);

  /* Stop shooting arrows if we killed the right monster */
  if (targetmonster == (m+'A'-1))
    { darkturns = 0; darkdir = NONE; targetmonster = 0; }

  goalr = goalc = NONE;			/* Clear old movement goal */
  monkilled[m]++; totalkilled++;	/* Bump kill count */
  hitstokill = mhit = mmiss = 0;	/* Clear indiviual monster stats */
  mtarget = NONE;			/* Clear target */
  beingheld = cancelled = 0;		/* Clear flags */

  /* If we killed an invisible, assume no more invisible around */
  if (!cosmic && !blinded &&
      (streq (monster, "invisible stalker") || streq (monster, "phantom")))
    beingstalked = 0;
}

/*
 * washit: Record being hit by a monster.
 */

washit (monster)
char *monster;
{
  register int mh = 0, m = 0;

  /* Find out what really hit us */
  if ((mh = getmonhist (monster, 1)) != NONE)
    { monster = monhist[mh].m_name; m = monsternum (monster); }

  dwait (D_MONSTER, "Was hit by a '%s'", monster);

  timeshit++;			/* Bump global count */

  if (m>0) wakemonster(-m);	/* Wake him up */

  terpbot ();			/* Hit points changed, read bottom */

  /* Add data about the event to long term memory */
  if (mh != NONE) {
    addprob (&monhist[mh].theyhit, SUCCESS);
    addstat (&monhist[mh].damage, lastdamage);
    analyzeltm ();
  }
}

/*
 * wasmissed: Record being missed by a monster.
 */

void wasmissed (monster)
char *monster;
{
  register int mh = 0, m = 0;

  /* Find out what really missed us */
  if ((mh = getmonhist (monster, 1)) != NONE)
    { monster = monhist[mh].m_name; m = monsternum (monster); }

  dwait (D_MONSTER, "Was missed by a '%s'", monster);

  timesmissed++;		/* Bump global count */

  if (m>0) wakemonster(-m);	/* Wake him up */

  /* Add data to long term memory */
  if (mh != NONE) {
    addprob (&monhist[mh].theyhit, FAILURE);
    analyzeltm ();
  }
}

/*
 * didhit: Record hitting a monster.
 */

void didhit ()
{
  register int m = 0;

  /* Record our hit */
  if (!cosmic) m = lastmonster;

  hits++; hitstokill++;
  addprob (&monhist[monindex[m]].wehit, SUCCESS);

  if (wielding (wand))
    { inven[currentweapon].charges--; newweapon++; }
}

/*
 * didmiss: Record missing a monster.
 */

void didmiss ()
{
  register int m = 0;

  /* Record our miss */
  if (!cosmic) m = lastmonster;

  misses++;
  addprob (&monhist[monindex[m]].wehit, FAILURE);

  if (usingarrow && goodarrow < 20)
    { newweapon = badarrow = 1; remember (currentweapon, WORTHLESS); }
}

/*
 * mshit: Record hitting a monster with a missile.
 */

void mshit (monster)
char *monster;
{
  register int mh;

  /* Arching in a dark room? */
  if (!cosmic && !blinded && targetmonster > 0 && streq (monster, "it"))
    monster = monname (targetmonster);

  /* Add data about the event to long term memory */
  if ((mh = getmonhist (monster, 0)) < 0) return;

  {
    addprob (&monhist[monindex[mh]].arrowhit, SUCCESS);

    if (mh == mtarget) { mhit++; }
    else { mhit=1; mmiss = 0; mtarget=mh; }
  }
}

/*
 * msmiss: Record missing a monster with a missile.
 */

void msmiss (monster)
char *monster;
{
  register int mh;

  /* Arching in a dark room? */
  if (!cosmic && !blinded && targetmonster > 0 && streq (monster, "it"))
    monster = monname (targetmonster);

  /* Add data about the event to long term memory */
  if ((mh = getmonhist (monster, 0)) < 0) return;

  {
    addprob (&monhist[monindex[mh]].arrowhit, FAILURE);

    if (mh == mtarget) { mmiss++; }
    else { mmiss=1; mhit=0; mtarget=mh; }
  }
}

/*
 * Countgold: called whenever msg contains a message about the number
 *            of gold pieces we just picked up. This routine keeps
 *            statistics about the amount of gold picked up.
 */

void countgold (amount)
register char *amount;
{
  int pot;

  if ((pot = atoi (amount)) > 0)
    { sumgold += pot; sumsqgold += pot*pot; numgold ++; }
}

/*
 * Summary: print a summary of the game.
 */

void summary (f, sep)
FILE *f;
char sep;
{
  register int m;
  char s[2048], *monname (char);

  sprintf (s, "Monsters killed:%c%c", sep, sep);

  for (m=0; m<=26; m++)
    if (monkilled[m] > 0) {
      sprintf (s, "%s\t%d %s%s%c", s, monkilled[m],  monname (m+'A'-1),
               plural (monkilled[m]), sep);
    }

  sprintf (s, "%s%cTotal: %d%c%c", s, sep, totalkilled, sep, sep);

  sprintf (s, "%sHit %d out of %d times, was hit %d out of %d times.%c", s,
           hits, misses+hits,
           timeshit, timesmissed+timeshit, sep);

  if (numgold > 0)
    sprintf (s, "%sGold %d total, %d pots, %d average.%c", s,
             sumgold, numgold, (sumgold*10+5) / (numgold*10), sep);
  sprintf(s, "%sSeed: %d, Genotype: %d %d %d %d %d %d %d %d.%c", s,
      g_seed, knob[0], knob[1], knob[2], knob[3],
      knob[4], knob[5], knob[6], knob[7], sep);

  if (f == NULL)
    addstr (s);
  else
    fprintf (f, "%s", s);
}

/*
 * versiondep: Set version dependent variables.
 */

void versiondep ()
{
  if (version_has_new_monsters())		genocide = "DMJGU";
  else if (version >= RV52A)	genocide = "UDVPX";
  else				genocide = "UXDPW";

  analyzeltm ();
}

/*
 * getmonhist: Retrieve the index in the history array of a monster,
 * taking our status into account.  This code is responsible for determining
 * when we are being stalked by an invisible monster.
 */

int getmonhist (monster, hitormiss)
char *monster;
int hitormiss;
{
  if (cosmic || blinded)
    { return (findmonster ("it")); }
  else {
    if (streq (monster, "it") && hitormiss) {
      if (version_has_invisible_stalker()) {
        if (! seemonster ("invisible stalker")) beingstalked=INVHIT;

        return (findmonster ("invisible stalker"));
      }
      else {
        if (! seemonster ("phantom")) beingstalked=INVHIT;

        return (findmonster ("phantom"));
      }
    }
    else {
      if (version < RV52B && streq (monster, "invisible stalker") &&
          ! seemonster (monster))
        beingstalked = INVHIT;

      return (findmonster (monster));
    }
  }
}
