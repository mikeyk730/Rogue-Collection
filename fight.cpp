//All the fighting gets done here
//@(#)fight.c          1.43 (AI Design)                1/19/85

#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <sstream>

using std::max;

#include "rogue.h"
#include "game_state.h"
#include "hero.h"
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
#include "pack.h"
#include "env.h"

char tbuf[MAXSTR];

const char* it = "it";
const char* you = "you";

// Each level is twice the previous
long e_levels[20] = { 10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 
  20480, 40960, 81920, 163840, 327680, 655360, 1310720, 2621440, 0 };

void do_hit(Item* weapon, int thrown, Agent* monster, const char* name)
{
  bool did_huh = false;

  if (thrown) 
    display_throw_msg(weapon, name, "hits", "hit");
  else 
    display_hit_msg(NULL, name);

  if (weapon && weapon->type==POTION)
  {
    affect_monster(weapon, monster);
    if (!thrown)
    {
      if (--weapon->count == 0) {
        player.pack.remove(weapon);
        delete weapon;
      }
      set_current_weapon(NULL);
    }
  }

  if (player.can_confuse())
  {
    did_huh = true;
    monster->set_confused(true);
    player.set_can_confuse(false);
    msg("your hands stop glowing red");
  }

  if (monster->get_hp() <= 0)
    killed(monster, true);
  else if (did_huh && !player.is_blind()) 
    msg("the %s appears confused", name);
}

void do_miss(Item* weapon, int thrown, Agent* monster, const char* name)
{
  if (thrown)
    display_throw_msg(weapon, name, "misses", "missed");
  else
    miss(NULL, name);
  if (monster->can_divide() && rnd(100)>25)
    slime_split(monster);
}

//fight: The player attacks the monster.
int fight(Coord *location, Item *weapon, bool thrown)
{
    const char *name;
    //Find the monster we want to fight
    Agent *monster = monster_at(*location);
    if (!monster)
        return false;

    //Since we are fighting, things are not quiet so no healing takes place.  Cancel any command counts so player can recover.
    repeat_cmd_count = 0;
    turns_since_heal = 0;

    start_run(monster);
    //Let him know it was really a mimic (if it was one).
    if (monster->is_disguised() && !player.is_blind())
    {
        monster->disguise = monster->type;
        if (thrown)
            return false;
        msg("wait! That's a %s!", monster->get_monster_name());
    }
    name = player.is_blind() ? it : monster->get_monster_name();

    if (roll_em(&player, monster, weapon, thrown) || (weapon && weapon->type == POTION))
    {
        do_hit(weapon, thrown, monster, name);
        return true;
    }

    do_miss(weapon, thrown, monster, name);
    return false;
}

bool aquator_attack()
{
  //If a rust monster hits, you lose armor, unless that armor is leather or there is a magic ring
  if (get_current_armor() && get_current_armor()->get_armor_class() < 9 && get_current_armor()->which != LEATHER)
    if (is_wearing_ring(R_SUSTARM))
      msg("the rust vanishes instantly");
    else {
      msg("your armor weakens, oh my!"); 
      get_current_armor()->weaken_armor();
      return true;
    }
    return false;
}

void ice_monster_attack()
{
  //When an Ice Monster hits you, you get unfrozen faster
  if (sleep_timer>1) 
      sleep_timer--;
}

bool rattlesnake_attack()
{
  //Rattlesnakes have poisonous bites
  if (!save(VS_POISON))
    if (!is_wearing_ring(R_SUSTSTR)) {
        player.adjust_strength(-1);
      msg("you feel a bite in your leg%s", noterse(" and now feel weaker"));
      return true;
    }
    else 
      msg("a bite momentarily weakens you");

    return false;
}

void flytrap_attack(Agent* mp)
{
  //Flytrap stops the poor guy from moving
  player.set_is_held(true);
  std::ostringstream ss;
  ss << ++(mp->value) << "d1";
  mp->stats.damage = ss.str();
}

// return true if attack succeeded
bool leprechaun_attack(Agent* mp)
{
  //Leprechaun steals some gold
  long lastpurse;

  lastpurse = game->hero().get_purse();
  game->hero().adjust_purse(-rnd_gold());
  if (!save(VS_MAGIC)) 
    game->hero().adjust_purse(-(rnd_gold()+rnd_gold()+rnd_gold()+rnd_gold()));
  if (game->hero().get_purse() != lastpurse) 
    msg("your purse feels lighter");

  return true;
}

bool nymph_attack(Agent* mp)
{
    const char *she_stole = "she stole %s!";

    //Nymphs steal a magic item, look through the pack and pick out one we like.
    Item* item = NULL;
    int nobj = 0;
    for (auto it = player.pack.begin(); it != player.pack.end(); ++it) {
        Item* obj = *it;
        if (obj != get_current_armor() && obj != get_current_weapon() &&
            obj != get_ring(LEFT) && obj != get_ring(RIGHT) &&
            is_magic(obj) && rnd(++nobj) == 0)
            item = obj;
    }
    if (item == NULL)
        return false;

    if (item->count > 1 && item->group == 0)
    {
        int oc;
        oc = item->count--;
        item->count = 1;
        msg(she_stole, inv_name(item, true));
        item->count = oc;
    }
    else {
        player.pack.remove(item);
        msg(she_stole, inv_name(item, true));
        delete item;
    }
    return true;
}

bool vampire_wraith_attack(Agent* monster)
{
    //Wraiths might drain energy levels, and Vampires can steal max_hp
    if (rnd(100) < (monster->drains_exp() ? 15 : 30)) // vampires are twice as likely to connect
    {
        int damage;

        if (monster->drains_exp())
        {
            if (player.stats.exp == 0) 
                death(monster->type); //All levels gone
            if (--player.stats.level == 0) { 
                player.stats.exp = 0; 
                player.stats.level = 1; 
            }
            else 
                player.stats.exp = e_levels[player.stats.level - 1] + 1;
            damage = roll(1, 10);
        }
        else
            damage = roll(1, 5); //vampires only half as strong

        player.stats.max_hp -= damage;
        if (player.stats.max_hp < 1)
            death(monster->type);
        player.decrease_hp(damage, false);
        msg("you suddenly feel weaker");
        return true;
    }
    return false;
}

//attack: The monster attacks the player
bool attack(Agent *monster)
{
  const char *name;
  int monster_died = false;
  bool attack_success = false; // todo:set this everywhere

  //Since this is an attack, stop running and any healing that was going on at the time.
  running = false;
  repeat_cmd_count = turns_since_heal = 0;
  if (monster->is_disguised() && !player.is_blind()) 
    monster->disguise = monster->type;
  name = player.is_blind() ? it : monster->get_monster_name();
  if (roll_em(monster, &player, NULL, false))
  {
      display_hit_msg(name, NULL);
      if (player.get_hp() <= 0)
          death(monster->type); //Bye bye life ...

      //todo: modify code, so enemy can have more than one power
      if (!monster->powers_cancelled()) {
          if (monster->hold_attacks())
          {
              flytrap_attack(monster);
          }
          else if (monster->shoots_ice())
          {
              ice_monster_attack();
          }
          else if (monster->rusts_armor())
          {
              attack_success = aquator_attack();
          }
          else if (monster->steals_gold())
          {
              attack_success = leprechaun_attack(monster);
          }
          else if (monster->steals_magic())
          {
              attack_success = nymph_attack(monster);
          }
          else if (monster->drains_strength())
          {
              attack_success = rattlesnake_attack();
          }
          else if (monster->drains_life() || monster->drains_exp())
          {
              attack_success = vampire_wraith_attack(monster);
          }

          if (attack_success && monster->dies_from_attack())
          {
              monster_died = true;
              remove_monster(monster, false);
          }
      }
  }
  else if (!monster->shoots_ice())
  {
      if (monster->hold_attacks())
      {
          if (!player.decrease_hp(monster->value, true))
              death(monster->type); //Bye bye life ...
      }
      miss(name, NULL);
  }
  clear_typeahead_buffer();
  repeat_cmd_count = 0;
  status();

  return !monster_died;
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
    player.increase_hp(add, false, false);
    if (use_level_names())
        msg("and achieve the rank of \"%s\"", level_titles[i - 1]);
    else
        msg("Welcome to level %d", i);
  }
}

//roll_em: Roll several attacks
bool roll_em(Agent *thatt, Agent *thdef, Item *weapon, bool hurl)
{
  struct Agent::Stats *att, *def;
  const char *cp;
  int ndice, nsides, def_arm;
  bool did_hit = false;
  int hplus;
  int dplus;
  int damage;

  att = &thatt->stats;
  def = &thdef->stats;
  if (weapon==NULL) {
      cp = att->damage.c_str();
      dplus = 0; 
      hplus = 0;
  }
  else
  {
    hplus = weapon->get_hit_plus();
    dplus = weapon->get_damage_plus();
    //Check for vorpally enchanted weapon
    if (weapon->is_vorpalized_against(thdef)) {
        hplus += 4; 
        dplus += 4;
    }
    if (weapon==get_current_weapon())
    {
      if (is_ring_on_hand(LEFT, R_ADDDAM))
          dplus += get_ring(LEFT)->get_ring_level();
      else if (is_ring_on_hand(LEFT, R_ADDHIT)) 
          hplus += get_ring(LEFT)->get_ring_level();
      if (is_ring_on_hand(RIGHT, R_ADDDAM)) 
          dplus += get_ring(RIGHT)->get_ring_level();
      else if (is_ring_on_hand(RIGHT, R_ADDHIT))
        hplus += get_ring(RIGHT)->get_ring_level();
    }
    cp = weapon->damage;
    if (hurl && weapon->is_missile() && get_current_weapon() && get_current_weapon()->which == weapon->launcher)
    {
      cp = weapon->throw_damage;
      hplus += get_current_weapon()->get_hit_plus();
      dplus += get_current_weapon()->get_damage_plus();
    }
    //Drain a staff of striking
    if (weapon->type == STICK && weapon->which == WS_HIT)
    {
        if (weapon->get_charges() == 0)
            cp = "0d0";
        weapon->drain_striking();
    }
  }
  //If the creature being attacked is not running (asleep or held) then the attacker gets a plus four bonus to hit.
  if (!thdef->is_running()) 
      hplus += 4;
  def_arm = def->ac;
  if (def==&player.stats)
  {
    if (get_current_armor()!=NULL) def_arm = get_current_armor()->get_armor_class();
    if (is_ring_on_hand(LEFT, R_PROTECT)) def_arm -= get_ring(LEFT)->get_ring_level();
    if (is_ring_on_hand(RIGHT, R_PROTECT)) def_arm -= get_ring(RIGHT)->get_ring_level();
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
      if (thdef==&player && max_level()==1) damage = (damage+1)/2;
      thdef->decrease_hp(max(0, damage), true);
      did_hit = true;
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
  else if (player.is_blind()) strcpy(tbuf, it);
  else {strcpy(tbuf, "the "); strcat(tbuf, who);}
  if (upper) *tbuf = toupper(*tbuf);
  return tbuf;
}

//hit: Print a message to indicate a successful hit
void display_hit_msg(const char *er, const char *ee)
{
  char *s;

  addmsg(prname(er, true));
  switch ((short_msgs())?1:rnd(4))
  {
  case 0: s = " scored an excellent hit on "; break;
  case 1: s = " hit "; break;
  case 2: s = (er==0?" have injured ":" has injured "); break;
  case 3: s = (er==0?" swing and hit ":" swings and hits "); break;
  }
  msg("%s%s", s, prname(ee, false));
}

//miss: Print a message to indicate a poor swing
void miss(const char *er, const char *ee)
{
  char *s;

  addmsg(prname(er, true));
  switch ((short_msgs())?1:rnd(4))
  {
  case 0: s = (er==0?" swing and miss":" swings and misses"); break;
  case 1: s = (er==0?" miss":" misses"); break;
  case 2: s = (er==0?" barely miss":" barely misses"); break;
  case 3: s = (er==0?" don't hit":" doesn't hit"); break;
  }
  msg("%s %s", s, prname(ee, false));
}

//save_throw: See if a creature save against something
int save_throw(int which, Agent *monster)
{
  int need = 14 + which - monster->stats.level/2;
  return (roll(1, 20) >= need);
}

//save: See if he saves against various nasty things
int save(int which)
{
  if (which==VS_MAGIC)
  {
    if (is_ring_on_hand(LEFT, R_PROTECT)) 
        which -= get_ring(LEFT)->get_ring_level();
    if (is_ring_on_hand(RIGHT, R_PROTECT)) 
        which -= get_ring(RIGHT)->get_ring_level();
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
void display_throw_msg(Item *item, const char *name, char *does, char *did)
{
  if (item->type == WEAPON)
    addmsg("the %s %s ", get_weapon_name(item->which), does);
  else 
    addmsg("you %s ", did);
  player.is_blind() ? msg(it) : msg("the %s", name);
}

//remove: Remove a monster from the screen
void remove_monster(Agent *monster, bool waskill)
{
    Coord* monster_pos = &monster->pos;
    for (auto it = monster->pack.begin(); it != monster->pack.end();){
        Item* obj = *(it++);
        obj->pos = monster->pos;
        monster->pack.remove(obj);
        if (waskill)
            fall(obj, false);
        else
            delete(obj);
    }
    if (Level::get_tile(*monster_pos) == PASSAGE)
        standout();
    if (monster->oldch == FLOOR && !can_see(monster_pos->y, monster_pos->x))
        Screen::DrawChar(*monster_pos, ' ');
    else if (monster->oldch != MDK)
        Screen::DrawChar(*monster_pos, monster->oldch);
    standend();

    level_monsters.remove(monster);
    delete monster;
}

//is_magic: Returns true if an object radiates magic
bool is_magic(Item *obj)
{
  switch (obj->type)
  {
  case ARMOR: return obj->get_armor_class()!=get_default_class(obj->which);
  case WEAPON: return obj->get_hit_plus() !=0 || obj->get_damage_plus() !=0;
  case POTION: case SCROLL: case STICK: case RING: case AMULET: return true;
  }
  return false;
}

//killed: Called to put a monster to death
void killed(Agent *monster, bool print)
{
    player.stats.exp += monster->stats.exp;

    //If the monster was a flytrap, un-hold him
    if (monster->can_hold()) {
        player.set_is_held(false);
    }
    else if (monster->drops_gold()) {
        int value = rnd_gold();
        if (save(VS_MAGIC))
            value += rnd_gold() + rnd_gold() + rnd_gold() + rnd_gold();
        Item *gold = new Gold(value);
        monster->pack.push_front(gold);
    }
    if (print)
    {
        addmsg("you have defeated ");
        if (player.is_blind())
            msg(it);
        else msg("the %s", monster->get_monster_name());
    }
    //Do adjustments if he went up a level
    check_level();
    //Get rid of the monster.
    remove_monster(monster, true);
}
