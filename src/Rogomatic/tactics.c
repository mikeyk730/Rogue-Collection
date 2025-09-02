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
 * tactics.c:
 *
 * This file contains all of the 'medium level intelligence' of Rog-O-Matic.
 */

# include <stdio.h>
# include <ctype.h>
# include <curses.h>
# include "types.h"
# include "globals.h"
# include "install.h"

/*
 * handlearmor: This routine is called to determine whether we should
 * take off or put on armor.
 *
 * Current strategy:   Wear best armor on levels 1..7 or 19 on or
 *                     if protected or have maintain armor.
 *                     Wear 2nd best armor between levels 13..18.
 *                     Wear best leather armor between levels 8..12
 *                     or 2nd best if no leather armor.   DR UTexas 12/15/83
 *
 * Note that leather armor does not rust.
 */

int handlearmor ()
{
  int obj;

  /* Only check when armor status is different */
  if (!newarmor || cursedarmor) return (0);

  /*
   * Pick the armor we want to wear. If we are worried about rust monster
   * we wear the second best armor, but if we wont see any rust monsters,
   * if our armor is too good for a rust monster to hit it, or we have a
   * ring of maintain armor, then we should wear our best armor.  On
   * levels 13-18 we wear our second best no matter what.
   */

  obj = havearmor (1, NOPRINT, ANY);		/* Get best armor */

  if (Level > (rust_monster_level()) && Level < 19 &&
      wearing ("maintain armor") == NONE &&
      willrust (obj) &&
      itemis (obj, KNOWN)) {
    obj = NONE;

    if (Level<13)		obj = havearmor (1, NOPRINT, RUSTPROOF);

    if (Level<13 && obj==NONE)	obj = havearmor (3, NOPRINT, ANY);

    if (obj==NONE)		obj = havearmor (2, NOPRINT, ANY);
  }

  /* If  the new armor is really bad, then don't bother wearing any */
  if (obj != NONE && armorclass (obj) > 9 && itemis (obj, KNOWN))
    { obj = NONE; }

  /* If we are wearing the right armor, then don't bother */
  if (obj == currentarmor)
    { newarmor = 0; return (0); }

  /* Debugging */
  dwait (D_PACK, "handlearmor: obj %d, currentarmor %d", obj, currentarmor);

  /* Take off the wrong armor */
  if (currentarmor != NONE && takeoff ())
    { return (1); }

  /* Put on the right armor, avoid wearing cursed armor */
  if (obj != NONE)
    { return (wear (obj)); }

  /* If we have no armor, then forget it */
  newarmor = 0;
  return (0);
}

/*
 * handleweapon: wield our best weapon. Calls haveweapon.
 *
 *  The current strategy is to wield the best weapon from haveweapon.
 */

int handleweapon ()
{
  int obj;

  if ((!newweapon || cursedweapon) && !wielding (thrower)) return (0);

  /* haveweapon (1) returns the index of the best weapon in the pack */
  if ((obj = haveweapon (1, NOPRINT)) == NONE) return (0);

  /* If we are not wielding our best weapon, do so */
  if (obj == currentweapon)	{ newweapon = 0; return (0); }
  else if (obj != NONE)		{ return (wield (obj)); }
  else				{ newweapon = 0; return (0); }
}

/*
 * quaffpotion: check whether we should quaff a potion, and call
 * quaff if so.  We quaff add strength, restore strength, healing,
 * extra healing, and raise level here.  Potions of seeinvisible
 * are handled in 'fightinvisible'.
 *
 * If we are at or below the exp. level, then experiment with unknown potions.
 */

int quaffpotion ()
{
  int obj = NONE, obj2 = NONE;

  /* Take advantage of double haste bug -- assures permanent haste */
  if (!doublehasted && has_double_haste_bug() &&
      ((hasted && (obj = havenamed (potion, "haste self")) != NONE) ||
       ((obj = havemult (potion, "haste self", 2)) != NONE)) &&
      quaff (obj))
    return (1);

  /*
   * Can we use a gain strength to our advantage? Or a restore?
   * If we have a Gain Strength, or our strength is very bad,
   * then we quaff a Regain Strength.
   */

  if (Str == Strmax && (obj = havenamed (potion, "gain strength")) != NONE &&
      quaff (obj))
    return (1);

  if ((Str < 700 ||
       (Str != Strmax && (havenamed (potion, "gain strength") != NONE))) &&
      (obj = havenamed (potion, "restore strength")) != NONE &&
      quaff (obj))
    return (1);

  if ((Str < 1600 || Level > 12) &&
      (obj = havemult (potion, "restore strength", 2)) != NONE &&
      quaff (obj))
    return (1);

  /* Try to get unblinded by quaffing a potion */
  if (blinded &&
      ((obj = havenamed (potion, "healing")) != NONE ||
       (obj = havenamed (potion, "extra healing")) != NONE ||
       (obj = havenamed (potion, "see invisible")) != NONE) &&
      quaff (obj))
    return (1);

  /* Try to get uncosmic by quaffing a potion */
  if (cosmic &&
      (obj = havenamed(potion, "extra healing")) != NONE &&
      quaff (obj))
    return (1);

  if (cosmic && Str != Strmax &&
      (obj = havenamed (potion, "poison")) != NONE) {
    if (wearing ("sustain strength") != NONE && quaff (obj) ||
        findring ("sustain strength"))
      return (1);
  }

  /*
   * Quaff healing to raise our MaxHp
   * Wait for cosmic known to quaff extra healing. DR,TG  UTexas
   */

  if (Hp == Hpmax &&
      ((obj = havemult (potion, "healing", 2)) != NONE ||
       (obj = havemult (potion, "extra healing", 2)) != NONE ||
       know ("blindness") && (obj = havenamed (potion, "healing")) != NONE ||
       know ("blindness") && (know ("hallucination") || !version_has_hallucination())  &&
       Level < 15 && (obj = havenamed (potion, "extra healing")) != NONE) &&
      quaff (obj))
    return (1);

  /*
   * Quaff a raise level potion?
   */

  if ((Explev > 8 || Level > 13) &&
      (obj = havenamed (potion, "raise level")) != NONE &&
      quaff (obj))
    return (1);

  /* Quaff an unknown potion? */
  if ((Level >= (k_exper/10) || objcount >= maxobj || Str<1000 || blinded) &&
      (obj = unknown (potion)) != NONE) {
    if ((obj2 = wearing ("add strength")) != NONE && removering (obj2))
      return (1);
    else if (wearing ("sustain strength") == NONE &&
             (obj2 = havenamed (ring, "sustain strength")) != NONE &&
             puton (obj2))
      return (1);
    else if (quaff (obj))
      return (1);
  }

  return (0);
}

/*
 * readscroll: check whether we should read a scroll, and call reads
 * to actually read it.
 *
 * Scrolls of identify, remove curse, genocide, enchant weapon,
 * enchant armor, magic mapping are defined.  Scrolls of scare
 * monster and confuse monster are handled in 'battlestations'.
 *
 * If we are at or below (k_exper/10), experiment with unknown scrolls.
 * Make certain that we are wearing our best armor when reading
 * enchant armor or an unknown scroll (which could be enchant
 * armor).
 */

int readscroll ()
{
  register int obj, obj2;

  /* Check the item specific identify scrolls first */
  if (((obj = havenamed (Scroll, "identify scroll")) != NONE &&
       (obj2 = unknown (Scroll)) != NONE) ||
      ((obj = havenamed (Scroll, "identify armor")) != NONE &&
       (obj2 = unknown (armor)) != NONE) ||
      ((obj = havenamed (Scroll, "identify weapon")) != NONE &&
       (obj2 = unknown (hitter)) != NONE) ||
      ((obj = havenamed (Scroll, "identify potion")) != NONE &&
       (obj2 = unknown (potion)) != NONE) ||
      ((obj = havenamed (Scroll, "identify ring, wand or staff")) != NONE &&
       ((obj2 = unknown (ring)) != NONE || (obj2 = unknown (wand)) != NONE))) {
    prepareident (obj2, obj);
    return (reads (obj));
  }

  /* In older version, have multiple uses for generic identify scrolls

  Note:
  "unknown" means the KNOWN trait isn't set. See inventory() to see when it is set.
  "unidentified" is an "unknown" item that we've used before, but still couldn't infer

  General identification preferences:
    1. The weapon we're carrying if it's unknown
       - todo:mdk: if we can track vorpalized weapons, we could id to find target monster
    2. Unknown rings
    3. Wands we've tried before, but couldn't infer
    4. Scrolls we've tried before, but couldn't infer
    5. Unknown wands (if level 11+)
    6. If the game doesn't have wands (v3.6), we'll consider unknown potion and any scrolls

    mdk: observations:
    - We never id scrolls without trying them first, so we'll always waste the first scare monster.
    Interestingly in 3.6, we won't even give preference to a unknown scroll over a scroll we've
    already identified.
    - We have an inference db. It maps codename -> actual name, and records if we've tried the item.
    Once we identify an item, the inference db is no longer relevant for that item.
    - We don't seem to track known things. If the known item isn't actually in our inventory,
    we can't use that in our decision making. Even if we've identified all scrolls except for scare
    monster, we'd still have to read one to discover that
    - Everything about an item is parsed from the inventory list, so we can't track traits that don't
    show in inventory text, e.g. did we enchant this weapon? is this weapon vorpalized? for which monster?
    - Cursed rings aren't tracked. We only wear rings that are known to be good
    - Potions are never thrown or wielded
  */
  int have_vorpal_to_id = did_read_vorpal && !did_id_vorpal;
  if ((obj = havenamed (Scroll, "identify")) != NONE &&
      (currentweapon != NONE) &&
      (have_vorpal_to_id ||
      (!itemis (currentweapon, KNOWN) && (!usingarrow || goodarrow > 20))))
  {
    if (have_vorpal_to_id)
    {
        did_id_vorpal = 1;
    }

    prepareident (currentweapon, obj);
    return (reads (obj));
  }

  if ((obj = havenamed (Scroll, "identify")) != NONE &&
      ((obj2 = unknown (ring)) != NONE ||
       (obj2 = unidentified (wand)) != NONE ||
       (obj2 = unidentified (Scroll)) != NONE ||
       Level > 10 && (obj2 = unknown (wand)) != NONE ||
       ((cheat || !version_has_wands()) &&
        ((obj2 = unknown (potion)) != NONE ||
         (obj2 = haveother (Scroll, obj)) != NONE)))) {
    prepareident (obj2, obj);
    return (reads (obj));
  }

  if ((cursedarmor || cursedweapon) &&
      (obj = havenamed (Scroll, "remove curse")) != NONE)
    return (reads (obj));

  if ((obj = havenamed (Scroll, "genocide")) != NONE)
    return (reads (obj));

  if (currentweapon != NONE &&
      (goodweapon || usingarrow || MaxLevel > 12) &&
      (obj = havenamed (Scroll, "enchant weapon")) != NONE)
    return (reads (obj));

  if (Level != didreadmap && Level > 12 &&
      (obj = havenamed (Scroll, "magic mapping")) != NONE)
    return (reads (obj));

  /* About to read an unknown scroll. We will assure that we have */
  /* a weapon in hand, and put on our best armor for the occasion */
  /* We must also prepare to identify something, just in case.    */

  if ((obj = havenamed (Scroll, "enchant armor")) != NONE ||
      (obj = havenamed (Scroll, "protect armor")) != NONE ||
      ((currentweapon != NONE) &&
       (Level >= (k_exper/10) || objcount >= maxobj ||
        cursedarmor || cursedweapon) &&
       (exploredlevel || Level > 18 || know ("aggravate monsters")) &&
       (obj = unknown (Scroll)) != NONE)) {
    prepareident (pickident (), obj);

    /* Go to a corner to read the scroll */
    if (read_in_corner() && !know ("create monster") && gotocorner ())
      return (1);

    /* Must put on our good armor first */
    if (!cursedarmor &&
        (!know("enchant armor") || stlmatch(inven[obj].str, "enchant armor") ||
         !know("protect armor") || stlmatch(inven[obj].str, "protect armor"))) {
      int obj2 = havearmor (1, NOPRINT, ANY); /* Pick our best armor */

      if (obj2 == currentarmor);

      /* Take off the bad stuff */
      else if (currentarmor != NONE && takeoff ()) return (1);

      /* Put on the good stuff */
      else if (obj2 != NONE && wear (obj2)) return (1);
    }

    /* No armor handling, so read the scroll */
    return (reads (obj));
  }

  return (0);
}

/*
 * handlering: check whether we should put on a ring, and call
 * puton to wear it.  Calls 'havering' to find the two best rings
 * and wears them if their evaluations are greater than 1000.
 *
 * 'havering' understands about when different rings are good, and how
 * much food we need to use each ring.
 */

int handlering ()
{
  int ring1, ring2;

  if (!newring && !beingstalked) return (0);

  ring1 = havering (1, NOPRINT);
  ring2 = havering (2, NOPRINT);

  dwait (D_PACK, "Handlering: ring1 %d, ring2 %d, left %d, right %d",
         ring1, ring2, leftring, rightring);

  if ((leftring == ring1 && rightring == ring2) ||
      (rightring == ring1 && leftring == ring2)) {
    newring = 0; return (0);
  }

  if (leftring != NONE && leftring != ring1 && leftring != ring2 &&
      removering (leftring)) {
    return (1);
  }

  if (rightring != NONE && rightring != ring1 && rightring != ring2 &&
      removering (rightring)) {
    return (1);
  }

  if (ring1 != leftring && ring1 != rightring && puton (ring1)) {
    return (1);
  }

  if (ring2 != leftring && ring2 != rightring && puton (ring2)) {
    return (1);
  }

  return (0);
}

/*
 * findring: called with the named of a ring, attempts to locate such
 * a ring in the pack and wear it. It will remove rings (other than
 * maintain armor) to accomplish this task if it we are wearing two
 * rings.
 *
 * Could be extended to have an ordering of rings to wear.
 */

int findring (name)
char *name;
{
  int obj;

  if ((obj = havenamed (ring, name)) == NONE ||
      wearing (name) != NONE)
    return (0);

  if (leftring != NONE && rightring != NONE) {
    if (stlmatch (inven[leftring].str, "maintain armor"))
      return (removering (rightring));
    else
      return (removering (leftring));
  }

  return (puton (obj));
}

/*
 * grope: get to a safe square and sit and vibrate (move back and forth)
 * and then sleep for 'turns' turns.
 *
 * Problem: We need to know which side of us the monster is on. Then
 * we could zap him with wands or staves.  This requires some kind of
 * memory and the ability to detect when the motion command (ie 'hit'
 * fails to move us).		MLM
 */

int grope (turns)
register int turns;
{
  register int k, moves;

  if (atrow < 2 || atcol < 1) {
    command ("grope", T_GROPING, "%ds", (turns > 0) ? turns : 1);
    return (1);
  }

  /* Count adjacent CANGO squares */
  for (k=0, moves=0; k<8; k++)
    if (onrc(CANGO, atdrow(k), atdcol(k))) moves++;

  if (moves > 2 && findsafe ()) /* find a spot with 2 or fewer moves */
    return (1);

  /* blindir is direction of adjacent CANGO square which is not a trap */
  for (k=0; k<4; k++, blindir = (blindir+2) % 8)
    if ((onrc(CANGO|TRAP, atdrow(blindir), atdcol(blindir)) == CANGO))
      break;

  if (turns) command ("grope 2", T_GROPING, "%c%c%ds", keydir[blindir],
                        keydir[(blindir+4)&7], turns);
  else       command ("grope 3", T_GROPING, "%c%c", keydir[blindir],
                        keydir[(blindir+4)&7]);

  blindir = (blindir+2) % 8;
  return (1);
}

/*
 * findarrow: This function tries to run over an arrow trap to get a
 *            magic arrow. Make certain we have some food.
 */

findarrow ()
{
  /* If wrong version, not cheating or must go find food, then forget it */
  if (!version_has_arrow_bug() || !cheat || hungry())
    return (0);

  else if (!usingarrow && foundarrowtrap && !on (ARROW) &&
           gotowards (trapr, trapc, 0))
    { display ("Trying for arrow..."); return (1); }

  return (0);
}

/*
 * checkcango: verify that a missile fired in direction 'dir' will
 *             travel 'turns' turns.
 *
 * Modified by mlm, 5/31/83: Return false if a monster is in the way.
 * only return true if the missile will travel EXACTLY the distance
 * specified.  Also changed it to not check the current square (since
 * we can fire from a door, even if we cant shoot through one).
 */

int checkcango (dir, turns)
register int dir, turns;
{
  register int r, c, dr, dc;

  for (dr = deltr[dir], dc = deltc[dir], r=atrow+dr, c=atcol+dc;
       turns > 0 && onrc (CANGO | DOOR, r, c) == CANGO;
       r+=dr, c+=dc, turns--)
    ;

  return (turns==0);
}

/*
 * godownstairs: issues a down command and check for the halftimeshow.
 */

int godownstairs (running)
register int running; /* True ==> don't do anything fancy */
{
  register int p;

  /* We don't want to go down if we have just gotten an arrow, since */
  /* It is probably bad, and we will want to go back to the trap;   */
  /* Don't go down until we have killed five monsters in one blow.   */
  /* While waiting, run back and forth to look for monsters.        */

  if (cheat && version_has_arrow_bug() && !running &&
      foundarrowtrap && usingarrow &&
      have (food) != NONE && goodarrow < 5 && waitaround ()) {
    saynow ("Checking out arrow...");
    return (1);
  }

  /* Check for applicability of this rule */
  if (! new_stairs) return (0);

  /* If we are on the stairs, perhaps we should rest up some */
  p = between ((Explev+larder)*10, 60, 100);

  if (atrow == stairrow && atcol == staircol &&
      !running && larder > 0 && Hp < max (10, percent (Hpmax, p))) {
    command ("rest on stairs", T_RESTING, "s");
    display ("Resting on stairs before next level");
    return (1);
  }

  /* Allow other rules a chance to notice that we are done with the level */
  if (on (STAIRS) && !exploredlevel)
    { exploredlevel = 1; return (1); }

  /* If we are floating, we cant go down, either rest or fail */
  if (floating && running)
    { saynow ("Cannot escape, floating in mid-air!"); return (0); }
  else if (floating) {
    saynow ("Floating above stairs...");
    command ("resting to stop floating", T_RESTING, "s"); return (1);
  }

  /* If we are on the stairs, go down */
  if (on (STAIRS)) {
    halftimeshow (Level);

    /* Start logging at Level GOODGAME, if we arent already */
    if (Level > (GOODGAME-2) && !replaying && !logging) toggleecho ();

    /* Send the DOWN command and return */
    command ("go down stairs", T_MOVING, ">");
    return (1);
  }

  /* If we are running and can run to the next level, do that */
  if (running && makemove (RUNDOWN, genericinit, downvalue, REEVAL)) {
    return (1);
  }

  /* If we see the stairs or a trap door, go there */
  if (!running && makemove (DOWNMOVE, genericinit, downvalue, REUSE)) {
    goalr = targetrow; goalc = targetcol;   /* Set a goal (CPU time hack) */
    return (1);
  }

  new_stairs = 0;
  return (0);
}

/*
 * plunge: Should we head down immediately?
 *
 * If we are being teleported too much or
 *    we are on a bad level (19 to 25) or
 *    we want to get past Rust Monsters (level 18) or
 *    we have aggravated all of the monsters then
 *
 * we head down immediately.
 */

int plunge ()
{
  /* Check for applicability of this rule */
  if (stairrow == NONE && !foundtrapdoor) return (0);

  if (have (amulet) != NONE) return (0);

  if (teleported > (larder+1)*5 && godownstairs (NOTRUNNING)) {
    if (!on (STAIRS)) saynow ("Giving up on level, too much teleporting");

    return (1);
  }

  if (Level > 17 && Level < 26 && godownstairs (NOTRUNNING)) {
    if (!on (STAIRS)) saynow ("Plunge mode!!!");

    return (1);
  }

  if (aggravated && godownstairs (NOTRUNNING)) {
    if (!on (STAIRS)) saynow ("Running from aggravated monsters");

    return (1);
  }

  if (haveexplored (9) && godownstairs (NOTRUNNING)) {
    if (!on (STAIRS)) saynow ("Level explored");

    return (1);
  }

  return (0);
}

/*
 * waitaround: Hang around here waiting for monsters.
 */

static struct {
  int vertstart,
      vertend,
      vertdelt,
      horstart,
      horend,
      hordelt;
} cb [4] = {
  {           3, (STATUSROW-2),  1,           1, (MAXCOLS-2),  1},	/* Top left corner */
  {           3, (STATUSROW-2),  1, (MAXCOLS-2),           1, -1},	/* Top right corner */
  { (STATUSROW-2),           3, -1, (MAXCOLS-2),           1, -1},	/* Bottom right corner */
  { (STATUSROW-2),           3, -1,           1, (MAXCOLS-2),  1}
};  /* Bottom left corner */

static gc = 0; /* Goal corner from 0..3 */

/*
 * waitaround: For some reason we want to stay on this level for a while.
 * Try running to each corner of the level.
 */

int waitaround ()
{
  register int i, j;

  if (gotowardsgoal ()) return (1);

  gc = ++gc % 4;

  for (i = cb[gc].vertstart; i != cb[gc].vertend; i += cb[gc].vertdelt)
    for (j = cb[gc].horstart; j != cb[gc].horend; j += cb[gc].hordelt)
      if (onrc (BEEN | CANGO | ROOM, i, j) &&
          !onrc (TRAP, i, j) && gotowards (i, j, 0))
        { goalr = i; goalc = j; return (1); }

  return (0);
}

/*
 * goupstairs:
 *
 *      If we have the amulet, and our score is good enough, then
 *      go up stairs. This function also checks for the end of the
 *      game, and issues the proper calls to get the score written.
 */

int goupstairs (running)
int running;
{
  int obj;

  /* Check for applicability of this rule */
  if (stairrow == NONE || have(amulet) == NONE ||
      (!running && quitat < BOGUS && Gold <= quitat))
    return (0);

  /* If we are on the stairs, then check for win, else go up */
  if (atrow == stairrow && atcol == staircol) {
    /* If we are about to win, dump any magic arrows or minus things */
    if (Level == 1 &&
        ((obj = havearrow ()) != NONE || (obj = haveminus ()) != NONE) &&
        throw (obj, 0))
      { return (1); }

    /* No magic arrows, time to leave */
    else if (Level == 1) {
      /* Send an up command and a space to clear the 'You Made It' */
      sendnow ("< ");

      /* Now read chars until we have the end of the inventory. */
      /* Note misspelling in Rogue 'Peices', so don't assume anything */
      waitfor ("Gold P");

      /* Note that quitrogue sends a '\n' to get the score */
      quitrogue ("total winner", Gold, 0);
      return (1);
    }

    /* Not at the top yet, keep on trucking */
    else
      { command ("go up stairs", T_MOVING, "<"); return (1); }
  }

  /* If we know where the stairs are, go there */
  else if ((goalr = stairrow) > 0 && (goalc = staircol) > 0 &&
           gotowards (goalr, goalc, running))
    return (1);

  return (0);
}

/*
 * restup: If we are low on hit points, sit for a while. Since handlering
 * was called first, we will be wearing a ring of regeneration if need be.
 *
 * First we find a good place to rest (we will move into a room, but not
 * out of one).  In lit rooms, stand far from doors so we can shoot
 * arrows at things coming in.  In dark rooms, stand diagonally away
 * from doors (so we get a one turn warning of monsters coming in that
 * door).  In either case, stand on stairs or next to trap doors and
 * teleport traps).
 *
 * Then rest by searching 's'.  If one blow would not kill us, and we
 * don't plan to shoot arrows, then rest up so as to heal one hit point.
 * If we are critically low, rest up one turn at a time.
 *
 * Other considerations:	Don't move if confused or cosmic.
 *				Drink healing potions if really low.
 *				Don't rest when hungry (and no food)
 */

int restup ()
{
  register int obj, turns;

  /* If we are confused, sit still so we don't bump into anything bad */
  if (confused) { command ("rest to clear confusion", T_RESTING, "s"); return (1); }

  /* If cosmic and plenty of hit points and food, rest for long periods */
  if (cosmic && (Hp >= percent (Hpmax, 80)) && larder > 2) {
    display ("Oh wow man, I'm contemplating my navel!");
    command ("rest to get rid of cosmic", T_RESTING, "100s"); return (1);
  }

  /* If we are well, return */
  if (Hp >= max (8, percent (Hpmax, between (Explev*10+k_rest-50, 40, 80))))
    { unrest ();  return (0); }

  /*
   * If we are really ill then try a healing potion (save a healing
   * potion for blindness, extra healing for hallucination).
   */

  if (Hp < Level+10 && Hp < Hpmax/3 &&
      ((obj = havemult (potion, "extra healing", 2)) != NONE ||
       (obj = havemult (potion, "healing", 2)) != NONE ||
       (know ("hallucination") &&
        (obj = havenamed (potion, "extra healing")) != NONE) ||
       (know ("blindness") &&
        (obj = havenamed (potion, "healing")) != NONE)) &&
      quaff (obj))
    { return (1); }

  /* Don't rest when we havent enough to eat */
  if (hungry ()) return (0);

  display ("Resting up...");

  /*
   * Look for a good place to rest
   */

  if (movetorest ()) return (1);

  /*
   * If we are very ill, or we are very deep, or we are in a lit room
   * and can shoot at things as they come ate us, rest only one turn so
   * monsters don't get the first shot. Otherwise rest enough turns
   * to heal one step.
   */

  turns = (Level < 8) ? (20-Explev*2) : 3;

  if ((!darkroom () && ammo) || Hp < Level*2+8 || Level > 15) turns = 1;

  command ("rest", T_RESTING, "%ds", turns);
  return (1);
}

/*
 * If goalr and goalc are set (not -1,-1) then attempt to move towards
 * that square. Calls gotowards which calls bfsearch.
 */

int gotowardsgoal ()
{
  if (goalr > 0 && goalc > 0) { /* Keep on trucking */
    if (goalr == atrow && goalc == atcol) { goalr = NONE; goalc = NONE; }
    else if (gotowards (goalr, goalc, 0)) { return (1); }
    else                                  { goalr = NONE; goalc = NONE; }
  }

  return (0);
}

/*
 * gotocorner:	Find a corner using downright and try to go there.
 *		This is done so we can destroy old wands by throwing
 *		them into the corner (which destroys them).
 */

int gotocorner ()
{
  int r, c;

  if (!downright (&r, &c)) return (0);

  if (debug (D_SCREEN))
    { saynow ("Gotocorner called:"); mvaddch (r, c, 'T'); at (row, col); }

  if (gotowards (r, c, 0)) { goalr=r; goalc=c; return (1); }

  return (0);
}

/*
 * lightroom: Try to light up the room if we are below level 17.
 */

int light ()
{
  if (Level < 17) return (0);

  return (lightroom ());
}

/*
 * shootindark: If we are arching at an old monster, fire another arrow.
 */

int shootindark ()
{
  register int obj, bow;

  /* If no longer arching in the dark, fail */
  if (darkturns < 1 || darkdir == NONE || !darkroom ()) return (0);

  darkturns--;			/* Count off turns till he reaches us */

  /* If he is one turn away, switch back to our sword */
  if (!cursedweapon && wielding (thrower) && darkturns==0 && handleweapon ())
    { dwait (D_BATTLE, "Switching to sword [4]"); return (1); }

  /* If we have room, switch to our bow */
  if (!cursedweapon && !wielding (thrower) && darkturns > 3 &&
      (bow = havebow (1, NOPRINT)) != NONE && wield (bow))
    return (1);

  /* Fail if we have run out of arrows */
  if ((obj = havemissile ()) == NONE) return (0);

  /* Throw the arrow in the arching direction */
  return (throw (obj, darkdir));
}

/*
 * dinnertime: Eat if we are hungry or if we have a surplus of food.
 */

int dinnertime ()
{
  if ((havefood (5) && objcount == maxobj && ! droppedscare) ||
      (larder > 0 && hungry ()))
    { return (eat ()); }

  return (0);
}

/*
 * trywand: Zap a blank wall with an unknown and unused wand in an attempt
 * to generate a message which identifies the wand.
 */

int trywand ()
{
  register int obj, dir, r, c, count;

  /* If we aren't in a room, if there are monsters around,  */
  /* or we are in the dark, then we can't try this strategy */
  if (!on (ROOM) || mlistlen || darkroom ()) return (0);

  /* Have we a wand to identify? */
  if (((obj = unknown (wand)) == NONE) || (itemis (obj, WORTHLESS)))
    return (0);
  else if (used (inven[obj].str))
    return (0);

  /* Look for a wall either 3 or 4 away */
  for (dir = 0; dir < 8; dir += 2) {
    for (count = 0, r=atrow, c=atcol;
         onrc (CANGO | DOOR, r, c) == CANGO;
         r += deltr[dir], c += deltc[dir])
      count++;

    if (count == 4 || count == 5) break;	/* Found a likely wall */
  }

  /* If we couldnt find room, then fail */
  if (dir > 7) return (0);

  /* point the wand */
  return (point (obj, dir));
}

/*
 * eat: If we have food, eat it.
 */

int eat ()
{
  int obj;

  if ((obj = have (food)) != NONE) {
    command ("eat", T_HANDLING, "e%c", LETTER(obj));
    return (1);
  }

  return (0);
}
