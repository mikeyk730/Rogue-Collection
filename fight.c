//All the fighting gets done here
//@(#)fight.c          1.43 (AI Design)                1/19/85

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "rogue.h"
#include "fight.h"
#include "list.h"
#include "weapons.h"
#include "monsters.h"
#include "io.h"
#include "rip.h"
#include "things.h"
#include "main.h"
#include "curses.h"
#include "slime.h"
#include "chase.h"
#include "potions.h"
#include "misc.h"
#include "mach_dep.h"
#include "level.h"
#include "rings.h"
#include "thing.h"
#include "armor.h"

char tbuf[MAXSTR];

const char* it = "it";
const char* you = "you";

// Each level is twice the previous
long e_levels[20] = { 10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 
  20480, 40960, 81920, 163840, 327680, 655360, 1310720, 2621440, 0 };

//fight: The player attacks the monster.
int fight(Coord *mp, char mn, ITEM *weap, bool thrown)
{
  AGENT *tp;
  const char *mname;

  //Find the monster we want to fight
  if ((tp = monster_at(mp->y, mp->x))==0) return FALSE;
  //Since we are fighting, things are not quiet so no healing takes place.  Cancel any command counts so player can recover.
  count = quiet = 0;
  start_run(mp);
  //Let him know it was really a mimic (if it was one).
  if (tp->type=='X' && tp->disguise!='X' && !on(player, ISBLIND))
  {
    mn = tp->disguise = 'X';
    if (thrown) return FALSE;
    msg("wait! That's a Xeroc!");
  }
  mname = get_monster_name(mn);
  if (on(player, ISBLIND)) mname = it;
  if (roll_em(&player, tp, weap, thrown) || (weap && weap->type==POTION))
  {
    bool did_huh = FALSE;

    if (thrown) thunk(weap, mname, "hits", "hit");
    else hit(NULL, mname);
    if (weap && weap->type==POTION)
    {
      th_effect(weap, tp);
      if (!thrown)
      {
        if (weap->count>1) weap->count--;
        else {detach_item(&player.pack, weap); discard_item(weap);}
        cur_weapon = NULL;
      }
    }
    if (on(player, CANHUH))
    {
      did_huh = TRUE;
      tp->flags |= ISHUH;
      player.flags &= ~CANHUH;
      msg("your hands stop glowing red");
    }
    if (tp->stats.hp<=0) killed(tp, TRUE);
    else if (did_huh && !on(player, ISBLIND)) msg("the %s appears confused", mname);
    return TRUE;
  }
  if (thrown) thunk(weap, mname, "misses", "missed");
  else miss(NULL, mname);
  if (tp->type=='S' && rnd(100)>25) slime_split(tp);
  return FALSE;
}

void aquator_attack()
{
  //If a rust monster hits, you lose armor, unless that armor is leather or there is a magic ring
  if (cur_armor && cur_armor->armor_class < 9 && cur_armor->which != LEATHER)
    if (is_wearing_ring(R_SUSTARM))
      msg("the rust vanishes instantly");
    else {
      msg("your armor weakens, oh my!"); 
      cur_armor->armor_class++;
    }
}

void ice_monster_attack()
{
  //When an Ice Monster hits you, you get unfrozen faster
  if (no_command>1) no_command--;
}

void rattlesnake_attack()
{
  //Rattlesnakes have poisonous bites
  if (!save(VS_POISON))
    if (!is_wearing_ring(R_SUSTSTR)) {
      chg_str(-1); 
      msg("you feel a bite in your leg%s", noterse(" and now feel weaker"));
    }
    else 
      msg("a bite momentarily weakens you");
}

void flytrap_attack(AGENT* mp)
{
  //Flytrap stops the poor guy from moving
  player.flags |= ISHELD;
  sprintf(mp->stats.damage, "%dd1", ++flytrap_hit);
}

void leprechaun_attack(AGENT* mp)
{
  //Leprechaun steals some gold
  long lastpurse;

  lastpurse = purse;
  purse -= GOLDCALC;
  if (!save(VS_MAGIC)) 
    purse -= GOLDCALC+GOLDCALC+GOLDCALC+GOLDCALC;
  if (purse<0) 
    purse = 0;
  remove_mons(&mp->pos, mp, FALSE);
  if (purse!=lastpurse) 
    msg("your purse feels lighter");
}

void nymph_attack(AGENT* mp)
{
  //Nymphs steal a magic item, look through the pack and pick out one we like.
  ITEM *obj, *steal;
  int nobj;
  char *she_stole = "she stole %s!";

  steal = NULL;
  for (nobj = 0, obj = player.pack; obj!=NULL; obj = next(obj))
    if (obj!=cur_armor && obj!=cur_weapon && obj!=cur_ring[LEFT] && obj!=cur_ring[RIGHT] && is_magic(obj) && rnd(++nobj)==0) steal = obj;
  if (steal!=NULL)
  {
    remove_mons(&mp->pos, mp, FALSE);
    if (steal->count>1 && steal->group==0)
    {
      int oc;

      oc = steal->count--;
      steal->count = 1;
      msg(she_stole, inv_name(steal, TRUE));
      steal->count = oc;
    }
    else {detach_item(&player.pack, steal); discard_item(steal); msg(she_stole, inv_name(steal, TRUE));}
  }
}

void vampire_wraith_attack(int type)
{
  //Wraiths might drain energy levels, and Vampires can steal max_hp
  if (rnd(100)<(type=='W'?15:30))
  {
    int fewer;

    if (type=='W')
    {
      if (player.stats.exp==0) death('W'); //All levels gone
      if (--player.stats.level==0) {player.stats.exp = 0; player.stats.level = 1;}
      else player.stats.exp = e_levels[player.stats.level-1]+1;
      fewer = roll(1, 10);
    }
    else fewer = roll(1, 5);
    player.stats.hp -= fewer;
    player.stats.max_hp -= fewer;
    if (player.stats.hp<1) player.stats.hp = 1;
    if (player.stats.max_hp<1) death(type);
    msg("you suddenly feel weaker");
  }
}

//attack: The monster attacks the player
void attack(AGENT *monster)
{
  const char *name;

  //Since this is an attack, stop running and any healing that was going on at the time.
  running = FALSE;
  count = quiet = 0;
  if (monster->type=='X' && !on(player, ISBLIND)) 
    monster->disguise = 'X';
  name = on(player, ISBLIND) ? it : get_monster_name(monster->type);
  if (roll_em(monster, &player, NULL, FALSE))
  {
    hit(name, NULL);
    if (player.stats.hp <= 0) 
      death(monster->type); //Bye bye life ...
   
    if (!on(*monster, ISCANC)) {
      switch (monster->type)
      {
      case 'A': 
        aquator_attack();
        break;

      case 'I': 
        ice_monster_attack();
        break;

      case 'R': 
        rattlesnake_attack();
        break;

      case 'W': case 'V':
        vampire_wraith_attack(monster->type);
        break;

      case 'F': 
        flytrap_attack(monster);
        break;

      case 'L': 
        leprechaun_attack(monster);
        break;

      case 'N': 
        nymph_attack(monster);
        break;

      default: break;
      }
    }
  }
  else if (monster->type!='I')
  {
    if (monster->type=='F')
    {
      player.stats.hp -= flytrap_hit;
      if (player.stats.hp<=0) death(monster->type); //Bye bye life ...
    }
    miss(name, NULL);
  }
  flush_type();
  count = 0;
  status();
}

//swing: Returns true if the swing hits
bool swing(int at_lvl, int op_arm, int wplus)
{
  int res = rnd(20);
  int need = (20-at_lvl)-op_arm;

  return (res+wplus>=need);
}

//check_level: Check to see if the guy has gone up a level.
void check_level()
{
  int i, add, olevel;

  for (i = 0; e_levels[i]!=0; i++) if (e_levels[i]>player.stats.exp) break;
  i++;
  olevel = player.stats.level;
  player.stats.level = i;
  if (i>olevel)
  {
    add = roll(i-olevel, 10);
    player.stats.max_hp += add;
    if ((player.stats.hp += add)>player.stats.max_hp) player.stats.hp = player.stats.max_hp;
    msg("and achieve the rank of \"%s\"", he_man[i-1]);
  }
}

//roll_em: Roll several attacks
bool roll_em(AGENT *thatt, AGENT *thdef, ITEM *weap, bool hurl)
{
  struct Stats *att, *def;
  char *cp;
  int ndice, nsides, def_arm;
  bool did_hit = FALSE;
  int hplus;
  int dplus;
  int damage;

  att = &thatt->stats;
  def = &thdef->stats;
  if (weap==NULL) {cp = att->damage; dplus = 0; hplus = 0;}
  else
  {
    hplus = weap->hit_plus;
    dplus = weap->damage_plus;
    //Check for vorpally enchanted weapon
    if (thdef->type==weap->enemy) {hplus += 4; dplus += 4;}
    if (weap==cur_weapon)
    {
      if (is_ring_on_hand(LEFT, R_ADDDAM)) dplus += cur_ring[LEFT]->ring_level;
      else if (is_ring_on_hand(LEFT, R_ADDHIT)) hplus += cur_ring[LEFT]->ring_level;
      if (is_ring_on_hand(RIGHT, R_ADDDAM)) dplus += cur_ring[RIGHT]->ring_level;
      else if (is_ring_on_hand(RIGHT, R_ADDHIT))
        hplus += cur_ring[RIGHT]->ring_level;
    }
    cp = weap->damage;
    if (hurl && (weap->flags&ISMISL) && cur_weapon!=NULL && cur_weapon->which==weap->launcher)
    {
      cp = weap->throw_damage;
      hplus += cur_weapon->hit_plus;
      dplus += cur_weapon->damage_plus;
    }
    //Drain a staff of striking
    if (weap->type==STICK && weap->which==WS_HIT && --weap->charges<0)
    {
      cp = weap->damage = "0d0";
      weap->hit_plus = weap->damage_plus = 0;
      weap->charges = 0;
    }
  }
  //If the creature being attacked is not running (asleep or held) then the attacker gets a plus four bonus to hit.
  if (!on(*thdef, ISRUN)) hplus += 4;
  def_arm = def->ac;
  if (def==&player.stats)
  {
    if (cur_armor!=NULL) def_arm = cur_armor->armor_class;
    if (is_ring_on_hand(LEFT, R_PROTECT)) def_arm -= cur_ring[LEFT]->ring_level;
    if (is_ring_on_hand(RIGHT, R_PROTECT)) def_arm -= cur_ring[RIGHT]->ring_level;
  }
  for (;;)
  {
    ndice = atoi(cp);
    if ((cp = strchr(cp, 'd'))==NULL) break;
    nsides = atoi(++cp);
    if (swing(att->level, def_arm, hplus+str_plus(att->str)))
    {
      int proll;

      proll = roll(ndice, nsides);
      damage = dplus+proll+add_dam(att->str);
      //special goodies for the commercial version of rogue
      //make it easier on level one
      if (thdef==&player && max_level==1) damage = (damage+1)/2;
      def->hp -= max(0, damage);
      did_hit = TRUE;
    }
    if ((cp = strchr(cp, '/'))==NULL) break;
    cp++;
  }
  return did_hit;
}

//prname: The print name of a combatant
char *prname(const char *who, bool upper)
{
  *tbuf = '\0';
  if (who==0) strcpy(tbuf, you);
  else if (on(player, ISBLIND)) strcpy(tbuf, it);
  else {strcpy(tbuf, "the "); strcat(tbuf, who);}
  if (upper) *tbuf = toupper(*tbuf);
  return tbuf;
}

//hit: Print a message to indicate a successful hit
void hit(const char *er, const char *ee)
{
  char *s;

  addmsg(prname(er, TRUE));
  switch ((terse || expert)?1:rnd(4))
  {
  case 0: s = " scored an excellent hit on "; break;
  case 1: s = " hit "; break;
  case 2: s = (er==0?" have injured ":" has injured "); break;
  case 3: s = (er==0?" swing and hit ":" swings and hits "); break;
  }
  msg("%s%s", s, prname(ee, FALSE));
}

//miss: Print a message to indicate a poor swing
void miss(const char *er, const char *ee)
{
  char *s;

  addmsg(prname(er, TRUE));
  switch ((terse || expert)?1:rnd(4))
  {
  case 0: s = (er==0?" swing and miss":" swings and misses"); break;
  case 1: s = (er==0?" miss":" misses"); break;
  case 2: s = (er==0?" barely miss":" barely misses"); break;
  case 3: s = (er==0?" don't hit":" doesn't hit"); break;
  }
  msg("%s %s", s, prname(ee, FALSE));
}

//save_throw: See if a creature save against something
int save_throw(int which, AGENT *tp)
{
  int need;

  need = 14+which-tp->stats.level/2;
  return (roll(1, 20)>=need);
}

//save: See if he saves against various nasty things
int save(int which)
{
  if (which==VS_MAGIC)
  {
    if (is_ring_on_hand(LEFT, R_PROTECT)) which -= cur_ring[LEFT]->ring_level;
    if (is_ring_on_hand(RIGHT, R_PROTECT)) which -= cur_ring[RIGHT]->ring_level;
  }
  return save_throw(which, &player);
}

//str_plus: Compute bonus/penalties for strength on the "to hit" roll
int str_plus(unsigned int str)
{
  int add = 4;

  if (str<8) return str-7;
  if (str<31) add--;
  if (str<21) add--;
  if (str<19) add--;
  if (str<17) add--;
  return add;
}

//add_dam: Compute additional damage done for exceptionally high or low strength
int add_dam(unsigned int str)
{
  int add = 6;

  if (str<8) return str-7;
  if (str<31) add--;
  if (str<22) add--;
  if (str<20) add--;
  if (str<18) add--;
  if (str<17) add--;
  if (str<16) add--;
  return add;
}

//raise_level: The guy just magically went up a level.
void raise_level()
{
  player.stats.exp = e_levels[player.stats.level-1]+1L;
  check_level();
}

//thunk: A missile hit or missed a monster
void thunk(ITEM *weap, const char *mname, char *does, char *did)
{
  if (weap->type==WEAPON)
    addmsg("the %s %s ", get_weapon_name(weap->which), does);
  else addmsg("you %s ", did);
  if (on(player, ISBLIND)) msg(it);
  else msg("the %s", mname);
}

//remove: Remove a monster from the screen
void remove_mons(Coord *mp, AGENT *tp, bool waskill)
{
  ITEM *obj, *nexti;

  if (tp==NULL) return;
  for (obj = tp->pack; obj!=NULL; obj = nexti)
  {
    nexti = next(obj);
    bcopy(obj->pos, tp->pos);
    detach_item(&tp->pack, obj);
    if (waskill)
      fall(obj, FALSE);
    else 
      discard_item(obj);
  }
  if (get_tile(mp->y, mp->x)==PASSAGE) standout();
  if (tp->oldch==FLOOR && !cansee(mp->y, mp->x)) mvaddch(mp->y, mp->x, ' ');
  else if (tp->oldch!='@') mvaddch(mp->y, mp->x, tp->oldch);
  standend();
  detach_agent(&mlist, tp);
  discard_agent(tp);
}

//is_magic: Returns true if an object radiates magic
is_magic(ITEM *obj)
{
  switch (obj->type)
  {
  case ARMOR: return obj->armor_class!=get_default_class(obj->which);
  case WEAPON: return obj->hit_plus!=0 || obj->damage_plus!=0;
  case POTION: case SCROLL: case STICK: case RING: case AMULET: return TRUE;
  }
  return FALSE;
}

//killed: Called to put a monster to death
void killed(AGENT *tp, bool pr)
{
  player.stats.exp += tp->stats.exp;
  //If the monster was a flytrap, un-hold him
  switch (tp->type)
  {

  case 'F':
    player.flags &= ~ISHELD;
    f_restor();
    break;

  case 'L':
    {
      ITEM *gold;

      if ((gold = create_item(GOLD, 0))==NULL) return;
      gold->gold_value = GOLDCALC;
      if (save(VS_MAGIC)) gold->gold_value += GOLDCALC+GOLDCALC+GOLDCALC+GOLDCALC;
      attach_item(&tp->pack, gold);

      break;
    }

  }
  if (pr)
  {
    addmsg("you have defeated ");
    if (on(player, ISBLIND)) msg(it);
    else msg("the %s", get_monster_name(tp->type));
  }
  //Do adjustments if he went up a level
  check_level();
  //Get rid of the monster.
  remove_mons(&tp->pos, tp, TRUE);
}
