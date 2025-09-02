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
 * strategy.c:
 *
 * This file contains all of the 'high level intelligence' of Rog-O-Matic.
 */

# include <stdio.h>
# include <ctype.h>
# include <curses.h>
# include <string.h>
# include "types.h"
# include "globals.h"
# include "install.h"

/*
 * foughtmonster records whether we engaged in battle recently.  This
 * information is used to tell whether we should sit still, waiting for a
 * confused monster to come back, or to go on about our business.
 * DIDFIGHT is the number of turns to sit still after a battle.
 */

# define DIDFIGHT 3

/*
 * strategize: Run through each rule until something fires. Return 1 if an
 * action was taken, otherwise return 0 (and then play will read a command
 * from the user).
 */

int   strategize ()
{
  dwait (D_CONTROL, "Strategizing...");

  /* If replaying, instead of making an action, return the old one */
  if (replaying) return (replaycommand ());

  /* Clear any messages we printed last turn */
  if (msgonscreen) { at (0,0); clrtoeol (); msgonscreen = 0; at (row,col); }


  /* ----------------------- Production Rules --------------------------- */


  if (callitpending ())         /* We have this to do */
    return (1);

  if (fightmonster ())          /* We are under attack! */
    return (1);

  if (fightinvisible ())        /* Claude Raines! */
    return (1);

  if (tomonster ())             /* Go play with the pretty monster */
    return (1);

  if (shootindark ())           /* Shoot arrows in dark rooms */
    return (1);

  if (handleweapon ())		/* Play with the nice sword */
    { dwait (D_BATTLE, "Switching to sword [1]"); return (1); }

  if (light ())			/* Fiat lux! Especially if we lost */
    return (1);			/* a monster from view.		   */

  if (dinnertime ())            /* Soups on! */
    return (1);

  /*
   * These variables are short term memory.  Slowed and
   * cancelled are fuses which are disabled after a small
   * number of turns.
   */

  lyinginwait = 0;			/* No more monsters to wait for */

  if (foughtmonster) foughtmonster--;	/* Turns since fought monster */

  if (slowed) slowed--; 	        /* Turns since we slowed a monster */

  if (cancelled) cancelled--;		/* Turns since we zapped 'cancel' */

  if (beingheld) beingheld--;		/* Turns since held by a fungus */

  /* ---- End of short term memory modification ---- */

  if (dropjunk ())		/* Send it back */
    return (1);

  if (readscroll ())		/* Get out the reading glasses */
    return (1);			  /* Must come before handlearmor() */

  if (handlearmor ())		/* Play dressup */
    return (1);

  if (quaffpotion ())		/* Glug glug glug ... */
    return (1);			  /* Must come before handlering() */

  if (handlering ())		/* We are engaged! */
    return (1);

  if (blinded && grope (50))	/* Who turned out the lights */
    { display ("Blinded, groping..."); return (1); }

  if (aftermelee ())		/* Wait for lingering monsters */
    return (1);

  if (tostuff ())		/* Pick up the play pretty */
    return (1);

  if (restup ())		/* Yawn! */
    return (1);

  if (goupstairs (NOTRUNNING))	/* Up we go! Make sure that we get */
    return (1);			  /* a better rank on the board. */

  if (trywand ())		/* Try to use a wand */
    return (1);

  if (gotowardsgoal ())		/* Keep on trucking */
    return (1);

  if (exploreroom ())		/* Search the room */
    return (1);

  if (archery ())		/* Try to position for fight */
    return (1);

  if (pickupafter ())		/* Look for stuff dropped by arched mon */
    return (1);

  if (plunge ())		/* Plunge mode */
    return (1);

  if (findarrow ())		/* Do we have an unitialized arrow? */
    return (1);

  if (findroom ())		/* Look for another room */
    return (1);

  /*
   * 'attempt' records the number of times we have completely searched
   * this level for secret doors.  If attempt is greater than 0, then we
   * have failed once to find the stairs and go down.  If this happens
   * three times, there could be amonster sleeping on the stairs.  We set
   * the SLEEPER bit for each square with a sleeping monster.  Go find
   * such a monster and kill it to see whether (s)he was on the stairs).
   */

  if (attempt > 4 && makemove (ATTACKSLEEP, genericinit, sleepvalue, REUSE)) {
    display ("No stairs, attacking sleeping monster...");
    return (1);
  }

  if (Level>1 && larder>0 && doorexplore ())	/* Grub around */
    return (1);

  if (godownstairs (NOTRUNNING))		/* Down we go! */
    return (1);

  if ((Level<2 || larder<1) && doorexplore())   /* Grub around anyway */
    return (1);

  /*
   * If we think we are on the stairs, but arent, maybe they were moved
   * (ie we were hallucinating when we saw them last time).
   */

  if (on (STAIRS) && (atrow != stairrow || atcol != staircol))
    { dwait (D_ERROR, "Stairs moved!"); findstairs (NONE, NONE); return (1); }

  /*
   * If we failed to find the stairs, explore each possible secret door
   * another ten times.
   */

  while (attempt++ < MAXATTEMPTS) {
    timestosearch += max (3, k_door / 5);
    foundnew ();

    if (doorexplore ()) return (1);
  }

  /*
   * Don't give up, start all over!
   */

  newlevel ();
  display ("I would give up, but I am too stubborn, starting over...");
  return (grope (10));
}

/*
 * callitpending : When going through the inventory we found this to
 *                 do, but we wanted to wait until we were no longer
 *                 in the middle of something else to tag this item
 *                 with this name.
 */

int callitpending ()
{
  /*
   * mdk: this function used to send the name as part of the command,
   * but it didn't account for clearing the --More-- message. I had to
   * extract a function below to actually send the name.
   */
  if (pending_call_letter != ' ') {
    command ("internal: call item", T_OTHER, "c%c", pending_call_letter);
    return (1);
  }
  return (0);
}

/* mdk: added to fix "call it" logic. */
void finishcallit()
{
    if (pending_call_letter != ' ') {
        sendnow("%s\n", pending_call_name);
        pending_call_letter = ' ';
        memset(pending_call_name, '\0', NAMSIZ);
    }
    else {
        dwait(D_FATAL, "No object to call, %c %s", pending_call_letter, pending_call_name);
    }
}

static char buf[128];

const char* describe(const char* msg, const char* monster)
{
    memset(buf, 0, 128);
    strcat(buf, msg);
    strcat(buf, monster);
    return buf;
}


/*
 * fightmonster: looks for adjacent monsters. If found, it calls
 * battlestations to prepare for battle otherwise hacks with the
 * weapon already in hand.
 */

int   fightmonster ()
{
  register int i, rr, cc, mdir = NONE, mbad  = NONE, danger = 0;
  int  melee = 0, adjacent = 0, alertmonster = 0;
  int  wanddir = NONE, m = NONE, howmean;
  char mon, monc = ':', *monster;

  /* Check for adjacent monsters */
  for (i = 0; i < mlistlen; i++) {
    rr = mlist[i].mrow; cc = mlist[i].mcol;

    if (max (abs (atrow-rr), abs (atcol-cc)) == 1) {
      if (mlist[i].q != ASLEEP) {
        if (mlist[i].q != HELD || Hp >= Hpmax || !havefood (1)) {
          melee = 1;

          if (mlist[i].q == AWAKE) alertmonster = 1;
        }
      }
    }
  }

  if (!melee) return (0);               /* No one to fight */

  /* Loop to find worst monster and tally danger & number adjacent */
  for (i = 0; i < mlistlen; i++) {
    rr = mlist[i].mrow; cc = mlist[i].mcol;	/* Monster position */

    /*
     * If the monster is adjacent and is either awake or
     * we don't know yet whether he is asleep, but we havent
     * see any alert monsters yet.
     */

    if (max (abs (atrow-rr), abs (atcol-cc)) == 1 &&
        (alertmonster ? mlist[i].q == AWAKE :
         mlist[i].q != ASLEEP)) { /* DR Utexas 26 Jan 84 */
      mon = mlist[i].chr;		/* Record the monster type */
      monster = monname (mon);		/* Record the monster name */
      danger += maxhitchar(mon);	/* Add to the danger */

      /* If he is adjacent, add to the adj count */
      if (onrc (CANGO, rr, atcol) && onrc (CANGO, atrow, cc)) {
        adjacent++; howmean = isholder (monster) ? 10000 : avghit(i);

        /* If he is adjacent and the worst monster yet, save him */
        if (howmean > mbad) {
          wanddir = mdir = direc (rr-atrow, cc-atcol);
          monc = mon; m = i; mbad = howmean;
        }
      }

      /* If we havent yet a line of sight, check this guy out */
      else if (wanddir == NONE)
        { wanddir = direc (rr-atrow, cc-atcol); }

      /* Debugging breakpoint */
      dwait (D_BATTLE, "%c <%d,%d>, danger %d, worst %c(%d,%d), total %d",
             screen[rr][cc], rr-atrow, cc-atcol,
             danger, monc, mdir, mbad, adjacent);
    }
  }

  /*
   * The following variables have now been set:
   *
   * monc:      The letter of the worst monster we can hit
   * mbad:      Relative scale 0 to 26, how bad is (s)he
   * mdir:      Which direction to him/her
   * danger:    How many hit points can (s)he/they do this round?
   * wanddir:   Direction of worst monster, even if we cant move to it.
   */

  /*
   * Check whether the battlestations expert has a suggested action.
   */

  monster = monname (monc);

  if (battlestations (m, monster, mbad, danger, adjacent ? mdir : wanddir,
                      adjacent ? 1 : 2, alertmonster, max (1, adjacent)))
    {
      foughtmonster = DIDFIGHT;
      return (1);
    }

  /*
   * If we did not wait for him last turn, and he is not adjacent,
   * let him move to us (otherwise, he gets to hits us first).
   */

  if (!lyinginwait && !adjacent) {
    command (describe("lying in wait for ", monster), T_FIGHTING, "s");
    dwait (D_BATTLE, "Lying in wait...");
    lyinginwait = 1;
    foughtmonster = DIDFIGHT;
    return (1);
  }

  /* If we are here but have no direction, there was a bug somewhere */
  if (mdir < 0) {
    dwait (D_BATTLE, "Adjacent, but no direction known!");
    return (0);
  }

  /* If we could die this round, tell the user about it */
  if (danger >= Hp) display ("In trouble...");

  /* Well, nothing better than to hit the beast! Tell dwait about it */
  dwait (D_BATTLE, "Attacking %s(%d) direction %d (total danger %d)...",
         monster, mbad, mdir, danger);

  /* Record the monster type */
  lastmonster = monc-'A'+1;

  /* Move towards the monster (this causes us to hit him) */
  rmove(describe("battle: strike monster ", monster), 1, mdir, T_FIGHTING);
  lyinginwait = 0;
  foughtmonster = DIDFIGHT;
  return (1);
}

/*
 * tomonster: if we can see a monster (and either it is awake or we
 * think we can beat it) then pick the worst one, call battlestations,
 * and then call gotowards to move toward the monster. If the monster
 * is an odd number of turns away, sit once to assure initiative before
 * charging after him. Special case for sitting on a door.
 */

int   tomonster ()
{
  register int i, dist, rr, cc, mdir = NONE, mbad = NONE;
  int   closest, which, danger = 0, adj = 0, alert = 0;
  char  monc = ':', monchar = ':', *monster;

  /* If no monsters, fail */
  if (mlistlen==0)
    return (0);

  /*
   * Loop through the monsters, 'which' and 'closest' record the index
   * and distance of the closest monster worth fighting.
   */

  for (i = 0, which = NONE, closest = 999; i < mlistlen; i++) {
    dist = max (abs (mlist[i].mrow - atrow), abs (mlist[i].mcol - atcol));
    monchar = mlist[i].chr;

    /*
     * IF   we are not using a magic arrow OR
     *      we want to wake this monster up AND we can beat him OR
     *      he is standing near something we want and we will have to
     *        fight him anywhay
     * THEN consider fighting the monster.
     *
     * Don't pick fights with sleepers if cosmic.  DR UTexas 25 Jan 84
     */

    if (usingarrow || mlist[i].q == AWAKE ||
        (!cosmic && wanttowake (monchar) &&
         (avghit (i) <= 50 || (maxhit (i) + 50 - k_wake) < Hp)) ||
        (mlist[i].q == HELD && Hp >= Hpmax)) {
      if (dist == 1) { /* mdk:bugfix more accurate damage and adjacent count. Fixes regeneration ring deadlock. */
        danger += maxhit(i);		/* track total danger */
        adj++;				/* count number of monsters */
      }

      /* If he is the closest monster, save his index and distance */
      if (dist < closest)
        { closest = dist; which = i; monc = mlist[i].chr; mbad = avghit(i); }

      /* Or if he is meaner than another equally close monster, save him */
      else if (dist == closest && avghit(i) > avghit(which)) {
        dwait (D_BATTLE, "Chasing %c(%d) rather than %c(%d) at distance %d.",
               mlist[i].chr, avghit(i), mlist[which].chr,
               avghit(which), dist);

        closest = dist; which = i; monc = mlist[i].chr; mbad = avghit(i);
      }
    }
  }

  /* No monsters worth bothering, return failure */
  if (which < 0) return (0);

  /* Save the monsters location in registers */
  rr = mlist[which].mrow - atrow; cc = mlist[which].mcol - atcol;

  /* If the monster is on an exact diagonal, record direction */
  mdir = (rr==0 || cc==0 || abs(rr)==abs(cc)) ? direc (rr, cc) : -1;

  /* Get a string which names the monster */
  monster = monname (monc);

  /* Is the monster alert */
  alert = (mlist[which].q == AWAKE) ? 1 : 0;

  /* If 'battlestations' has an action, use that action */
  if (battlestations (which, monster, mbad, danger, mdir, closest, alert, adj))
    return (1);

  /* If he is an odd number of squares away, lie in wait for him */
  if ((closest&1) == 0 && !lyinginwait) {
    command ("waiting for monster, odd spaces", T_FIGHTING, "s");
    dwait (D_BATTLE, "Waiting for monster an odd number of squares away...");
    lyinginwait = 1;
    return (1);
  }

  /* "We have him! Move toward him!" */
  if (gotowards (mlist[which].mrow, mlist[which].mcol, 0)) {
    goalr = mlist[which].mrow; goalc = mlist[which].mcol;
    lyinginwait = 0;
    return (1);
  }

  /* Could not find a path to the monster, record failure */
  return (0);
}

/*
 * wanttowake is true for monsters without special attacks, such that the
 * expected damage from hits is a reasonable estimate of their vorpalness.
 * Some monsters are included here because we want to shoot arrows at them.
 */

int wanttowake(c)
char c;
{
  char *monster = monname (c);

  if (missedstairs)
    return (1);

  /*
   * If monster sleeping but won't wake up when we move around him,
   * return wanttowake as false.   DR UTexas 09 Jan 84
   */

  if (streq (monster, "centaur") ||
      streq (monster, "dragon") ||
      streq (monster, "floating eye") ||
      streq (monster, "ice monster") ||
      streq (monster, "leprechaun") ||
      streq (monster, "nymph") ||
      streq (monster, "wraith") ||
      streq (monster, "purple worm") )
    return (0);

  return (1);
}

/*
 * aftermelee:	called when we have just fought a monster, assures
 *		that it wasn't just a confused monster that backed
 *		away and might get a hit on us if we move. Now only
 *		used when we lost a monster from view.
 *
 *		Also rest if we are critically weak and have some food.
 */

aftermelee ()
{
  if (foughtmonster > 0) {
    lyinginwait = 1;
    command ("rest after battle", T_RESTING, "s");
    dwait (D_BATTLE, "Aftermelee: waiting for %d rounds.", foughtmonster);
    return (1);
  }

  /* If critically weak, rest up so traps won't kill us.  DR Utexas */
  if (Hp < 6 && larder > 0) {
    command ("recover from severe battle", T_RESTING, "s");
    display ("Recovering from severe beating...");
    return (1);
  }

  return (foughtmonster = 0);
}

int is_monster_vorpal_target(const char* monster)
{
  return vorpal_target && streq(monster, monname(vorpal_target));
}

/*
 * battlestations:
 *
 *      We are going into battle. Can we think of anything better to
 *      to than simply hacking at him with our weapon?
 */

# define die_in(n)	(Hp/n < danger*50/(100-k_run))
# define live_for(n)	(! die_in(n))

battlestations (m, monster, mbad, danger, mdir, mdist, alert, adj)
int m;			/* Monster index */
char *monster;          /* What is it? */
int mbad;               /* How bad is it? */
int danger;             /* How many points damage per round? */
int mdir;               /* Which direction (clear line of sight)? */
int mdist;              /* How many turns until battle? */
int alert;              /* Is he known to be awake? */
int adj;		/* How many attackers are there? */
{
  int obj, turns;
  static int stepback = 0;

  /* Ascertain whether we have a clear path to this monster */
  if (mdir != NONE && !checkcango (mdir, mdist))
    mdir = NONE;

  /* Number of turns is one less than distance (modified if we are hasted) */
  turns = hasted ? (mdist-1)*2 : (mdist-1);

  /* No point in wasting resources when we are invulnerable */
  if (on (SCAREM) &&
      (turns > 0 || confused) &&
      !streq(monster, "dragon") &&
      (Hp < percent (Hpmax, 95))) {
    command ("rest on scare monster", T_RESTING, "s");
    display ("Resting on scare monster");
    dwait (D_BATTLE, "Battlestations: resting, on scaremonster.");
    return (1);
  }

  /*
   * Take invisible stalkers into account,
   * fightmonster() and tomonster() cant see stalkers.
   */

  if (beingstalked > INVPRES) { turns = 0; danger += INVDAM; }

  /* Debugging breakpoint */
  dwait (D_BATTLE,
         "Battlestations: %s(%d), total danger %d, dir %d, %d turns, %d adj.",
         monster, mbad, danger, mdir, turns, adj);

  debuglog("Battlestations: %s(%d), total danger %d, dir %d, %d turns, %d adj.\n",
      monster, mbad, danger, mdir, turns, adj);

  /*
   * Switch back to our mace or sword?
   */

  if (live_for (1) && turns < 2 && wielding (thrower) && handleweapon ())
    { dwait (D_BATTLE, "Switching to sword [2]"); return (1); }

  /*
   * Don't waste magic when on a scare monster scroll
   */

  if (on (SCAREM) && !streq (monster, "dragon")) {
    dwait (D_BATTLE, "Battlestations: hitting from scaremonster.");
    return (0);
  }

  /*
   * If we were busy resting on the stairs and we see a monster, go down
   * Go on down if about to be attacked by a monster with an effective
   * magic attack.  DR UTexas 25 Jan 84
   */

  if (on(STAIRS) && ((Level>18 && Level<26) || exploredlevel) && !floating &&
      (die_in(5) ||
       ((seeawakemonster ("rattlesnake") || seeawakemonster ("giant ant")) &&
        (havenamed (ring, "sustain strength") < 0)) ||
       ((seeawakemonster ("aquator") || seeawakemonster ("rust monster")) &&
        turns < 2 && willrust (currentarmor) &&
        wearing ("maintain armor") == NONE) ||
       seeawakemonster ("medusa") || seeawakemonster ("umber hulk"))) {
    if (goupstairs (RUNNING) || godownstairs (RUNNING))
      return (1);
  }

  /*
   * Are healing potions worthwhile?
   */

  if (die_in (1) && Hpmax-Hp > 10 && turns > 0 &&
      ((obj = havenamed (potion, "extra healing")) != NONE ||
       (obj = havenamed (potion, "healing")) != NONE))
    return (quaff (obj));

  /*
   * Run away if we are sure of the direction and we are in trouble
   * Don't try to run if a fungi has ahold of us. If we are confused,
   * we will try other things, and we will decide to run later.
   * If we are on a door, wait until the monster is on us (that way
   * we can shoot arrows at him, if we want to).
   * Don't run away from Dragons!!!  They'll just flame you.
   */

  if (!confused && !beingheld && (!on(DOOR) || turns < 1) &&
      (!streq (monster, "dragon") || cosmic) && Hp+Explev < Hpmax &&
      ((die_in(1) || Hp <= danger + between (Level-10, 0, 10)) || chicken) &&
      runaway ()) {
    display ("Run away! Run away!");
    darkdir = NONE; darkturns = 0;
    return(1);
  }

  /*
   * Be clever when facing multiple monsters? mdk: or slime
   */

  if ((adj > 1 || streq (monster, "slime")) && !confused && !beingheld && !on (STAIRS | DOOR) &&
      backtodoor (turns))
    return (1);

  /*
   * stepback to see if he is awake.
   */

  if (!alert && !beingheld && !stepback && mdir != NONE &&
      turns == 0 && !on (DOOR | STAIRS)) {
    int rdir = (mdir+4)%8;

    if (onrc (CANGO | TRAP, atdrow(rdir), atdcol(rdir)) == CANGO)
      { move1 ("step back to see if awake", rdir); stepback = 7; return (1); }
  }

  if (stepback) stepback--;     /* Decrement turns until step back again */

  /*
   * Should we put on our ring of maintain armor?   DR UTexas 19 Jan 84
   */

  if (live_for (1) && currentarmor != NONE &&
      (leftring == NONE || rightring == NONE) &&
      (seemonster ("aquator") || seemonster ("rust monster")) &&
      willrust (currentarmor) &&
      wearing ("maintain armor") == NONE &&
      (obj = havenamed (ring, "maintain armor")) != NONE &&
      puton (obj))
    return (1);

  if (turns > 1 && live_for (2) && leftring != NONE && rightring != NONE &&
      (seemonster ("aquator") || seemonster ("rust monster")) &&
      wearing ("maintain armor") < 0 &&
      findring ("maintain armor"))
    return (1);

  /*
   * Should we put on our ring of sustain strength?  DR UTexas 19 Jan 84
   */

  if ((live_for (1) || turns > 0) &&
      (leftring == NONE || rightring == NONE) &&
      (seemonster ("giant ant") || seemonster ("rattlesnake")) &&
      wearing ("sustain strength") < 0 &&
      (obj = havenamed (ring, "sustain strength")) != NONE &&
      puton (obj))
    return (1);

  if ((live_for (2) || turns > 1) &&
      leftring != NONE && rightring != NONE &&
      (seemonster ("giant ant") || seemonster ("rattlesnake")) &&
      wearing ("sustain strength") < 0 &&
      findring ("sustain strength"))
    return (1);

  /*
   * Should we put on our ring of regeneration? Make sure we wont kill
   * ourselves trying to do it, by checking how many turns it will take to
   * get it on compared to the number of hits we can take.
   */

  /* Have a ring and a free hand, one turn */
  if (die_in (4) && (live_for (1) || turns > 0) &&
      (leftring == NONE || rightring == NONE) &&
      !(turns == 0 && (streq (monster, "rattlesnake") ||
                       streq (monster, "giant ant"))) &&
      wearing ("regeneration") < 0 &&
      (obj = havenamed (ring, "regeneration")) != NONE &&
      puton (obj))
    return (1);

  /* Have a ring and both hands are full, takes two turns */
  if (die_in (4) && (live_for (2) || turns > 1) &&
      leftring != NONE && rightring != NONE &&
      wearing ("regeneration") < 0 &&
      findring ("regeneration"))
    return (1);

  /*
   * Haste ourselves?
   */

  if (!hasted && version > RV36B && (turns > 0 || live_for (1)) &&
      die_in (2) && (obj = havenamed (potion, "haste self")) != NONE &&
      quaff (obj))
    return (1);

  /*
   * Confuse the poor beast?
   */

  if (die_in (2) && turns > 0 && !redhands &&
      ((obj = havenamed (Scroll, "monster confusion")) != NONE))
    return (reads (obj));

  /*
   * mdk: if we can zap with our vorpalized weapon, use it now!
   */
  if (die_in (1)
      && can_vorpal_zap(currentweapon)
      && is_monster_vorpal_target(monster)
      && point(currentweapon, mdir))
  {
    dwait(D_INFORM, "Vorpalize: Zap with weapon %d: %s", currentweapon, monster);
    did_vorpal_zap = 1;
    return (1);
  }

  /*
   * Put them all to sleep? This does us little good, since we cant
   * currently infer that we have a scroll of Hold Monster. But we
   * will read scrolls of identify on the second one.  Bug, this
   * does not put them to sleep, it just holds them in place.
   * We have a lot more programming to do here!!!!   Fuzzy
   */

  if (die_in (1) && (obj = havenamed (Scroll, "hold monster")) != NONE &&
      reads (obj)) {
    holdmonsters ();
    return (1);
  }

  /*
   * Drop a scare monster?
   */

  if (die_in (1) && !streq(monster, "dragon") &&
      (obj = havenamed (Scroll, "scare monster")) != NONE &&
      drop (obj)) {
    set (SCAREM);
    droppedscare++;
    return (1);
  }

  /*
   * Buy buy birdy!
   */

  if (die_in (1) && mdir != NONE && turns == 0 &&
      (obj = havewand ("teleport away")) != NONE &&
      ! (itemis (obj, WORTHLESS)) &&
      point (obj, mdir)) {
    if (streq (monster, "violet fungi")) beingheld = 0;

    if (streq (monster, "venus flytrap")) beingheld = 0;

    return (1);
  }

  /*
   * Eat dust, turkey!
   */

  if (die_in (1) && turns == 0 &&
      (obj = havenamed (Scroll, "teleportation")) != NONE) {
    beingheld = 0;
    return (reads (obj));
  }

  /*
   * The better part of valor...
   */

  if ((die_in (1) && turns == 0 || fainting ()) && quitforhonors ())
    return (1);

  /*
   * If we trust our magic arrow, give it a whirl
   */

  if (!confused && cheat && usingarrow && goodarrow > 10 && turns == 0)
    return (0);

  /*
   * Try to protect our armor from Rusties.
   */

  if (!cursedarmor && currentarmor != NONE &&
      (seeawakemonster ("rust monster") || seeawakemonster ("aquator")) &&
      live_for (1) &&
      !(cosmic && Level < 8) &&               /* DR UTexas 25 Jan 84 */
      willrust (currentarmor) &&
      wearing ("maintain armor") == NONE &&
      takeoff ())
    { return (1); }

  /*
   * Any life saving wands?
   */

  if (die_in (2) && Hp > 40 && turns < 3 &&
      !(streq (monster, "purple worm") || streq (monster, "jabberwock")) &&
      (obj = havewand ("drain life")) != NONE &&
      ! (itemis (obj, WORTHLESS))
     )
    return (point (obj, 0));

  if (mdir != NONE && die_in (2) &&
      (!cosmic || Level > 18) &&          /* DR UTexas 31 Jan 84 */
      (streq (monster, "dragon")     || streq (monster, "purple worm")   ||
       streq (monster, "jabberwock") || streq (monster, "medusa")        ||
       streq (monster, "xorn")       || streq (monster, "violet fungi")  ||
       streq (monster, "griffin")    || streq (monster, "venus flytrap") ||
       streq (monster, "umber hulk") || streq (monster, "black unicorn")) &&
      (obj = havewand ("polymorph")) != NONE &&
      ! (itemis (obj, WORTHLESS))
      )
    return (point (obj, mdir));

  /*
   * Any life prolonging wands?
   */

  if ((die_in (1) || (turns == 0 && streq (monster, "floating eye")) ||
       (turns == 0 && streq (monster, "ice monster"))) &&
      mdir != NONE && mdist < 6 && !on(DOOR) &&
      ((obj = havewand ("fire")) != NONE && !streq(monster, "dragon") ||
       (obj = havewand ("cold")) != NONE ||
       (obj = havewand ("lightning")) != NONE) &&
      ! (itemis (obj, WORTHLESS))
     )
    return (point (obj, mdir));

  if (die_in (2) && mdir != NONE && !slowed && (turns>0 || live_for (2)) &&
      (obj = havewand ("slow monster")) != NONE &&
      (slowed = 5) &&
      ! (itemis (obj, WORTHLESS))
     )
    return (point (obj, mdir));

  if (mdir != NONE && !cancelled && turns == 0 &&
      (streq (monster, "wraith") ||
       streq (monster, "vampire") ||
       streq (monster, "floating eye") ||
       streq (monster, "ice monster") ||
       streq (monster, "leprechaun") ||
       streq (monster, "violet fungi") ||
       streq (monster, "venus flytrap")) &&
      (obj = havewand ("cancellation")) != NONE &&
      (cancelled = 10) &&
      ! (itemis (obj, WORTHLESS))
     ) {
    if (streq (monster, "violet fungi") || streq (monster, "venus flytrap"))
      beingheld = 0;

    return (point (obj, mdir));
  }

  if (((die_in (3) && live_for (1)) ||
       (turns == 0 && streq (monster, "floating eye")) ||
       (turns == 0 && streq (monster, "ice monster"))) &&
      mdir != NONE &&
      (((obj = havewand ("magic missile")) != NONE && turns > 0) ||
       ((obj = havewand ("striking")) != NONE && turns == 0)) &&
      ! (itemis (obj, WORTHLESS))
     )
    return (point (obj, mdir));

  /*
   * Since we have no directional things, we will try to run even though
   * we are confused. Again, wait at door until the monster is on us.
   * Don't run away from dragons, they'll just flame you!!
   */

  if (confused && !beingheld && (!on(DOOR) || turns < 1) &&
      ! streq (monster, "dragon") &&
      ((die_in (1) && Hp+Explev/2+3 < Hpmax) || chicken) &&
      runaway ())
    { display ("Run away! Run away!"); return(1); }

  /*
   * We can live for a while, try to get to a position where we can run
   * away if we really get into trouble.  Don't run away from dragons,
   * they'll just flame you!!!
   */

  if (!confused && !beingheld && ! streq (monster, "dragon") &&
      (mdir < 0 || turns < 5) &&
      (((adj > 1 || live_for (1)) && die_in (4) && !canrun ())) &&
      unpin ())
    { display ("Unpinning!!!"); return(1); }

  /*
   * Light up the room if we are in combat.
   */

  if (turns > 0 && die_in (3) && lightroom ())
    return (1);

  /*
   * We arent yet in danger and can shoot at the old monster.
   */

  if ((live_for (5) || turns > 1) && shootindark ())
    return (1);

  /*
   * Try out an unknown wand?  Try shooting unknown wands at
   * rattlesnakes since they are such a pain.   DR UTexas  19 Jan 84
   */

  if (live_for (2) && (Level > 8 || streq (monster, "rattlesnake") ||
                       streq (monster, "giant ant")) &&
      mdir != NONE && on(ROOM) && mdist < 6 &&
      ((obj = unknown (wand)) != NONE) && (!used (inven[obj].str)) &&
      ! (itemis (obj, WORTHLESS))
     ) {
    point (obj, mdir);
    usesynch = 0;
    return (1);
  }

  /*
   * Wait to see if he is really awake.
   */

  if (!alert && !lyinginwait && turns > 0) {
    command ("rest to see if monster is awake", T_FIGHTING, "s");
    dwait (D_BATTLE, "Waiting to see if he is awake...");
    lyinginwait = 1;
    return (1);
  }

  /*
   * Archery: try to move into a better position, and after that, try to
   * shoot an arrow at the beast. Conserve arrows below SAVEARROWS.
   */

  if ((streq (monster, "leprechaun") ||
       streq (monster, "nymph") ||
       streq (monster, "floating eye") ||
       streq (monster, "ice monster") ||
       streq (monster, "giant ant") ||
       streq (monster, "rattlesnake") ||
       streq (monster, "wraith") ||
       streq (monster, "vampire") ||
       streq (monster, "centaur") ||   /* DR UTexas 21 Jan 84 */
       die_in (1+k_arch/20) || ammo > SAVEARROWS+5-k_arch/10) &&
      (obj = havemissile ()) != NONE) {
    /* Move into position */
    if ((!alert || mdir < 0) && turns > 0 && archmonster (m, 1))
      return (1);

    /* If in position */
    if (!on (HALL) && mdir != NONE && turns > 0) {
      int bow;

      /* Wield the bow if we have time */
      if (!cursedweapon && !wielding (thrower) && turns > 4 &&
          (bow = havebow (1, NOPRINT)) != NONE && wield (bow))
        return (1);

      /* And shoot! */
      throw (obj, mdir);
      return (1);
    }
  }

  /*
   * Switch back to our mace or sword?
   */

  if (!cursedweapon && wielding (thrower) && handleweapon ())
    { dwait (D_BATTLE, "Switching to sword [3]"); return (1); }

  /*
   * No bright ideas. Return and let the caller figure out what to do.
   */

  return (0);
}

/*
 * tostuff: if we see something to pick up, go to it. If our pack is full,
 * try to drop our least useful item. If pack is still full, fail.
 */

int tostuff ()
{
  register int i, closest, dist, w, worst, worstval;
  int   which, wrow, wcol;
  stuff what;

  /* If we don't see anything (or don't care), return failure */
  if (slistlen == 0 || Level == 1 && have (amulet) != NONE) return (0);

  /*
   * Now find the closest thing to pick up.  Don't consider things we have
   * already dropped (those squares have the USELESS bit set), unless we
   * have dropped a scroll of SCARE MONSTER, in which case we want our
   * pack to be full.  Don't be fooled by stairs when hallucinating.
   *
   * NOTE: Don't pick up the scaremonster scroll!!!    MLM
   *
   * scaremonster shouldn't be worth going to if we have enough Hp
   * to do something else...
   */

  for (i = 0, which = NONE, closest = 999; i < slistlen; i++) {
    if (!onrc (USELESS, slist[i].srow, slist[i].scol) ||
        (droppedscare && objcount < maxobj &&
         !onrc (SCAREM, slist[i].srow, slist[i].scol))) {
      dist = max (abs (slist[i].srow - atrow), abs (slist[i].scol - atcol));

      /* Ignore Junk */
      if (onrc (USELESS, slist[i].srow, slist[i].scol) &&
         (!onrc (SCAREM, slist[i].srow, slist[i].scol))) dist = ROGINFINITY;

      /* make scaremonster infinity when we don't need it */
      if (onrc (SCAREM, slist[i].srow, slist[i].scol))
         if (Hp > percent (Hpmax, 80))
           dist = ROGINFINITY;

      /* If this is the closest item, save its distance and index */
      if (dist < closest)
        { closest = dist; which = i; }
    }
  }

  /* Could not find anything worth picking up, return failure */
  if (which < 0) return (0);

  /* Found something, save its location and type in registers */
  what= slist[which].what; wrow= slist[which].srow; wcol= slist[which].scol;

  /* We can always pick up more gold */
  if (what == gold) return (gotowards (wrow, wcol, 0));

  /* Have space in our pack, go get it */
  if (objcount < maxobj) return (gotowards (wrow, wcol, 0));

  /* No space in pack and we cannot drop something here, fail */
  if (on (STUFF | DOOR | TRAP | STAIRS)) return (0);

  /* Must drop something, pick least valuable item to drop */
  for (worst = NONE, worstval = 9999, i = 0;   i < invcount;   i++) {
    if (inven[i].count && !itemis (i, INUSE) && (w = worth (i)) < worstval)
      { worst = i; worstval = w; }

    /* Once we have found a totally useless item, stop looking */
    if (worstval == 0) break;
  }

  /* Found an item, drop it */
  if (worst != NONE) return (drop (worst));

  /* Pack is full and we can't find something to drop, fail */
  return (0);
}

/*
 * fightinvisible: being hounded by unseen beasties, try something clever.
 */

fightinvisible ()
{
  char cmd[20]; register int dir, liberties = 0, lastdir, obj;

  /* Count down the time since we were last hit by a stalker */
  if (--beingstalked < 0)
    { return (beingstalked=0); }

  /* If we are in real trouble, we might want to quit */
  if (beingstalked > INVPRES && Hp < INVDAM && quitforhonors ())
    { return (1); }

  /* Can we teleport out of here? */
  if (Hp < INVDAM && beingstalked > INVPRES &&
      (obj = havenamed (Scroll, "teleport")) != NONE && reads (obj)) {
    beingstalked = INVPRES-1;
    return (1);
  }

  /* Can we quaff a potion of see invisible? */
  if ((obj = havenamed (potion, "see invisible")) != NONE && quaff (obj))
    { beingstalked = 0; return (1); }

  /* If we have some time, try putting on a ring of see invisible */
  if (Hp > (INVDAM * 3/2) && beingstalked > INVLURK &&
      findring ("see invisible"))
    { return (1); }

  /* If we can bail out to the next level, do so */
  if (((beingstalked < INVPRES  && Hp < (INVDAM * 2)) ||
       (beingstalked >= INVPRES && Hp < (INVDAM * 3))) &&
      godownstairs (RUNNING)) {
    display ("Running like hell from an invisible stalker...");
    return (1);
  }

  /* Nothing worth doing, but he is around somewhere */
  if (beingstalked <= INVPRES)
    return (0);

  /* Must fight him 'mano a mano', tell the user (who cant see him either) */
  display ("Fighting invisible stalker...");
  *cmd = '\0';

  /* Record the monster type (for didhit and didmiss, see mess.c) */
  if (version_has_invisible_stalker())
    lastmonster = ('I'-'A'+1);
  else
    lastmonster = ('P'-'A'+1);

  /* Count how many orthogonal moves we can make */
  for (dir=0; dir<8; dir++)
    if (atdrow(dir) > 0 && atdrow(dir) < STATUSROW &&
        onrc(CANGO, atdrow(dir), atdcol(dir)) &&
        onrc(CANGO, atdrow(dir), atcol) &&
        onrc(CANGO, atrow, atdcol(dir)))
      { liberties++; lastdir = dir; }

  /* If can only go two ways, then go back and forth (will hit) */
  if (liberties == 1 || liberties == 2) {
    command ("fight: back and forth", T_FIGHTING, "%c%c", keydir[lastdir], keydir[(lastdir + 4) & 7]);
    return (1);
  }

  /* Try to get away, usually gets to a square with only 2 liberties */
  else if (runaway ()) return (1);

  /* Else run two and then double back on him. If that will */
  /* not work, run in a circle (will hit one out of 4)      */
  for (dir=0; dir<8; dir += 2)
    if ((onrc(CANGO, atdrow(dir), atdcol(dir))) &&
        (onrc(CANGO, atrow+2*deltr[dir], atcol+2*deltc[dir])))
      break;

  if (dir > 7)	command ("battle tactic 1: run in circle", T_FIGHTING, "hjlk");
  else		command ("battle tactic 2", T_FIGHTING, "%c%c%c", keydir[dir],
                     keydir[dir], keydir[(dir+4)&7]);

  return (1);
}

/*
 * archery: Try to arch sleeping monsters.  The 'mtokill' attr keeps track
 * of how many arrows we want to be able to pump into a monster before we
 * decide to wake him up.  That means we must be that far away AND have
 * that many missiles in our pack.  This number can be modified by our hit
 * and damage bonuses.
 *
 * Note: some monsters are to wimpy archery, and some too mean.     MLM
 */

archery ()
{
  register int m, mtk;
  char *monster;

  for (m=0; m < mlistlen; m++) {	/* Find a sleeping monster */
    monster = monname (mlist[m].chr);

    /*
     * If he is not awake and either
     *   we are much stronger than he is or
     *   he is a known target for archery and
     * we have enough arrows to wipe this dude out and
     * we have food or he is a leprechaun and we arent hungry yet
     *
     * Then try calling archmonster to move to the right place.
     */

    if (mlist[m].q != AWAKE &&
        gplushit != NONE &&
        !(mlist[m].q == HELD && Hp < Hpmax)  &&	/* DR UTexas 26 Jan 84 */
        (maxhit(m) > Hp/3 ||
         streq (monster, "leprechaun")	  ||
         streq (monster, "nymph")	  ||
         streq (monster, "floating eye")  ||
         streq (monster, "giant ant")	  ||
         streq (monster, "rattlesnake")	  ||
         streq (monster, "centaur")	  ||
         streq (monster, "ice monster"))  &&
        (ammo >= (mtk = monatt[mlist[m].chr-'A'].mtokill - gplushit)) &&
        (larder > 0 ||
        ((streq (monster, "leprechaun") && !hungry ()) ||
         streq (monster, "nymph")))) {
      dwait (D_BATTLE, "Arching at %c at (%d,%d)",
             mlist[m].chr, mlist[m].mrow, mlist[m].mcol);

      if (archmonster (m, mtk)) return (1);

      dwait (D_BATTLE, "Archmonster failed in archery.");
    }
  }

  return (0);
}

/*
 * pickupafter: Go stand on square where the monster used to be.
 *              If (s)he left something behind (evens just arrows
 *		that missed) we will find it and pick it up.
 *
 * Bug:		Sometimes goes the long way around and doesnt see things.
 */

pickupafter ()
{
  /* If no goal */
  if (agoalr < 0 || agoalc < 0)
    return (0);

  /* If on goal */
  if (atrow == agoalr && atcol == agoalc) {
    agoalr = agoalc = NONE;
    return (0);
  }

  /* mdk:bugfix don't go to the target if it's a trap. Teleport trap can cause an infinite loop. */
  if (onrc(TRAP, agoalr, agoalc)) {
      return 0;
  }

  /* Else go for it */
  return (gotowards (agoalr, agoalc, 0));
}

/*
 * dropjunk: This doesnt just drop something.  It destroys it.
 *           When an object is thrown diagonally into a corner,
 *           Rogue can't find a place to put it, and the object is
 *           removed from the game.
 */

int dropjunk ()
{
  int obj;

  if ((obj = haveuseless ()) != NONE && (gotocorner () || throw (obj, 7)))
    return (1);

  return (0);
}

/*
 * quitforhonors: We are in mortal danger.  Do we want to quit?
 *
 * Strategy:	'quitat' is the score to beat (set in setup);
 *		If we will beat it anyway, don't quit.  If we
 *		wont beat it anyway, don't quit.  If we will just
 *		beat the score by quiting, then do so.
 *
 * Assumes a 10 percent death tax.
 */

quitforhonors ()
{
  if (Gold > quitat && (Gold-Gold/10) <= quitat) {
    quitrogue ("quit (scoreboard)", Gold, 0);
    return (1);
  }

  return (0);
}
