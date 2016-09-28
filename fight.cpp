//All the fighting gets done here
//@(#)fight.c          1.43 (AI Design)                1/19/85

#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>

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
#include "output_interface.h"
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
#include "monster.h"

char tbuf[MAXSTR];

const char* it = "it";
const char* you = "you";

void Hero::do_hit(Item* weapon, int thrown, Monster* monster, const char* name)
{
    bool did_confuse = false;

    if (thrown)
        display_throw_msg(weapon, name, "hits", "hit");
    else
        display_hit_msg(NULL, name);

    if (weapon && weapon->type == POTION)
    {
        affect_monster(weapon, monster);
        if (!thrown)
        {
            if (--weapon->count == 0) {
                pack.remove(weapon);
                delete weapon;
            }
            set_current_weapon(NULL);
        }
    }

    if (can_confuse())
    {
        did_confuse = true;
        monster->set_confused(true);
        set_can_confuse(false);
        msg("your hands stop glowing red");
    }

    if (monster->get_hp() <= 0)
        killed(monster, true);
    else if (did_confuse && !is_blind())
        msg("the %s appears confused", name);
}

void Hero::do_miss(Item* weapon, int thrown, Monster* monster, const char* name)
{
  if (thrown)
    display_throw_msg(weapon, name, "misses", "missed");
  else
    display_miss_msg(NULL, name);

  if (monster->can_divide() && rnd(100)>25)
    slime_split(monster);
}

//fight: The player attacks the monster.
Monster* Hero::fight(Coord *location, Item *weapon, bool thrown)
{
    std::string name;
    //Find the monster we want to fight
    Monster* monster = game->level().monster_at(*location);
    if (!monster)
        return 0;

    //Since we are fighting, things are not quiet so no healing takes place.  Cancel any command counts so player can recover.
    repeat_cmd_count = 0;
    game->turns_since_heal = 0;

    monster->start_run();
    //Let him know it was really a mimic (if it was one).
    if (monster->is_disguised() && !this->is_blind())
    {
        monster->disguise = monster->type;
        if (thrown)
            return 0;
        msg("wait! That's a %s!", monster->get_name().c_str());
    }
    name = this->is_blind() ? it : monster->get_name();

    if (attack(monster, weapon, thrown) || (weapon && weapon->type == POTION))
    {
        do_hit(weapon, thrown, monster, name.c_str());
        return monster;
    }

    do_miss(weapon, thrown, monster, name.c_str());
    return 0;
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
  if (game->sleep_timer>1) 
      game->sleep_timer--;
}

bool rattlesnake_attack()
{
    //Rattlesnakes have poisonous bites
    if (!save(VS_POISON))
        if (!is_wearing_ring(R_SUSTSTR)) {
            game->hero().adjust_strength(-1);
            msg("you feel a bite in your leg%s", noterse(" and now feel weaker"));
            return true;
        }
        else
            msg("a bite momentarily weakens you");

    return false;
}

void flytrap_attack(Monster* mp)
{
  //Flytrap stops the poor guy from moving
  game->hero().set_is_held(true);
  std::ostringstream ss;
  ss << ++(mp->value) << "d1";
  mp->stats.damage = ss.str();
}

// return true if attack succeeded
bool leprechaun_attack(Monster* mp)
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

bool nymph_attack(Monster* mp)
{
    const char *she_stole = "she stole %s!";

    //Nymphs steal a magic item, look through the pack and pick out one we like.
    Item* item = NULL;
    int nobj = 0;
    for (auto it = game->hero().pack.begin(); it != game->hero().pack.end(); ++it) {
        Item* obj = *it;
        if (obj != get_current_armor() && obj != get_current_weapon() &&
            obj != get_ring(LEFT) && obj != get_ring(RIGHT) &&
            obj->is_magic() && rnd(++nobj) == 0)
            item = obj;
    }
    if (item == NULL)
        return false;

    if (item->count > 1 && item->group == 0)
    {
        int oc;
        oc = item->count--;
        item->count = 1;
        msg(she_stole, item->inv_name(true));
        item->count = oc;
    }
    else {
        game->hero().pack.remove(item);
        msg(she_stole, item->inv_name(true));
        delete item;
    }
    return true;
}

bool vampire_wraith_attack(Monster* monster)
{
    //Wraiths might drain energy levels, and Vampires can steal max_hp
    if (rnd(100) < (monster->drains_exp() ? 15 : 30)) // vampires are twice as likely to connect
    {
        int damage;

        if (monster->drains_exp())
        {
            if (game->hero().experience() == 0)
                death(monster->type); //All levels gone
            game->hero().reduce_level();
            damage = roll(1, 10);
        }
        else
            damage = roll(1, 5); //vampires only half as strong

        game->hero().stats.max_hp -= damage;
        if (game->hero().stats.max_hp < 1)
            death(monster->type);
        game->hero().decrease_hp(damage, false);

        msg("you suddenly feel weaker");
        return true;
    }
    return false;
}

//attack: The monster attacks the player
Monster* Monster::attack_player()
{
    std::string name;
    bool attack_success = false; // todo:set this everywhere

    //Since this is an attack, stop running and any healing that was going on at the time.
    game->modifiers.m_running = false;
    repeat_cmd_count = game->turns_since_heal = 0;

    if (is_disguised() && !game->hero().is_blind())
        disguise = type;
    name = game->hero().is_blind() ? it : get_name();

    if (attack(&game->hero(), NULL, false))
    {
        display_hit_msg(name.c_str(), NULL);
        if (game->hero().get_hp() <= 0)
            death(type); //Bye bye life ...

        //todo: modify code, so enemy can have more than one power
        if (!powers_cancelled()) {
            if (hold_attacks())
            {
                flytrap_attack(this);
            }
            else if (shoots_ice())
            {
                ice_monster_attack();
            }
            else if (rusts_armor())
            {
                attack_success = aquator_attack();
            }
            else if (steals_gold())
            {
                attack_success = leprechaun_attack(this);
            }
            else if (steals_magic())
            {
                attack_success = nymph_attack(this);
            }
            else if (drains_strength())
            {
                attack_success = rattlesnake_attack();
            }
            else if (drains_life() || drains_exp())
            {
                attack_success = vampire_wraith_attack(this);
            }

            if (attack_success && dies_from_attack())
            {
                remove_monster(this, false);
                return this;
            }
        }
    }
    else if (!shoots_ice())
    {
        if (hold_attacks())
        {
            if (!game->hero().decrease_hp(value, true))
                death(type); //Bye bye life ...
        }
        display_miss_msg(name.c_str(), NULL);
    }

    clear_typeahead_buffer();
    repeat_cmd_count = 0;
    status();

    return 0;
}

//attempt_swing: Returns true if the swing hits
bool attempt_swing(int lvl, int defender_amr, int hplus)
{
    int roll = rnd(20);
    int got = roll + hplus;
    int need = (20 - lvl) - defender_amr;
    bool hit(got >= need);
    
    std::ostringstream ss;
    ss << (hit ? "HIT: " : "MISS: ") << got << " ? " << need
        << " (1d20=" << roll << " + hplus=" << hplus << ") ? (20 - lvl=" << lvl << " - amr=" << defender_amr << ")";
    game->log("battle", ss.str());

    return hit;
}

//attack: Roll several attacks
bool Agent::attack(Agent *defender, Item *weapon, bool hurl)
{
    std::string damage_string;
    int hplus;
    int dplus;
    calculate_roll_stats(defender, weapon, hurl, &hplus, &damage_string, &dplus);

    //If the creature being attacked is not running (asleep or held) then the attacker gets a plus four bonus to hit.
    if (!defender->is_running())
        hplus += 4;

    int defender_armor = defender->calculate_armor();

    std::ostringstream ss;
    ss << get_name() << "[hp=" << get_hp() << "] " << damage_string << " attack on " 
        << defender->get_name() << "[hp=" << defender->get_hp() << "]";
    game->log("battle", ss.str());

    bool did_hit = false;
    const char* cp = damage_string.c_str();
    for (;;)
    {
        int ndice = atoi(cp);
        if ((cp = strchr(cp, 'd')) == NULL) 
            break;
        int nsides = atoi(++cp);
        if (attempt_swing(stats.level, defender_armor, hplus + str_plus(calculate_strength())))
        {
            did_hit = true;

            int r = roll(ndice, nsides);
            int str_bonus = add_dam(calculate_strength());
            int damage = dplus + r + str_bonus;

            bool half_damage(defender == &game->hero() && max_level() == 1); //make it easier on level one
            if (half_damage) {
                damage = (damage + 1) / 2;
            }
            damage = max(0, damage);
            defender->decrease_hp(damage, true);

            std::ostringstream ss;
            ss << "damage=" << damage << " => " << defender->get_name() << "[hp=" << defender->get_hp() << "]: ("
                << damage_string << "=" << r << " + dplus=" << dplus << " + str_plus=" << str_bonus << ")" 
                << (half_damage ? "/2" : "");
            game->log("battle", ss.str());
        }
        if ((cp = strchr(cp, '/')) == NULL) break;
        cp++;
    }
    return did_hit;
}

//prname: The print name of a combatant
char *prname(const char *who, bool upper)
{
  *tbuf = '\0';
  if (who==0) strcpy(tbuf, you);
  else if (game->hero().is_blind()) strcpy(tbuf, it);
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

//display_miss_msg: Print a message to indicate a poor swing
void display_miss_msg(const char *er, const char *ee)
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
  return save_throw(which, &game->hero());
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

//thunk: A projectile hit or missed a monster
void display_throw_msg(Item *item, const char *name, char *does, char *did)
{
    std::string type = item->name();
    if (type.empty())
        type = get_weapon_name(item->which);

  if (item->type == WEAPON)
    addmsg("the %s %s ", type.c_str(), does);
  else 
    addmsg("you %s ", did);
  game->hero().is_blind() ? msg(it) : msg("the %s", name);
}

//remove: Remove a monster from the screen
void remove_monster(Monster* monster, bool waskill)
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
    if (game->level().get_tile(*monster_pos) == PASSAGE)
        game->screen().standout();
    if (monster->oldch == FLOOR && !game->hero().can_see(*monster_pos))
        game->screen().mvaddch(*monster_pos, ' ');
    else if (monster->oldch != MDK)
        game->screen().mvaddch(*monster_pos, monster->oldch);
    game->screen().standend();

    game->level().monsters.remove(monster);
    delete monster;
}

//is_magic: Returns true if an object radiates magic
bool Item::is_magic()
{
    switch (type)
    {
    case ARMOR:
        return get_armor_class() != get_default_class(which);
    case WEAPON:
        return get_hit_plus() != 0 || get_damage_plus() != 0;
    case POTION: case SCROLL: case STICK: case RING: case AMULET:
        return true;
    }
    return false;
}

//killed: Called to put a monster to death
void killed(Monster* monster, bool print)
{
    //If the monster was a flytrap, un-hold him
    if (monster->can_hold()) {
        game->hero().set_is_held(false);
    }

    if (monster->drops_gold()) {
        int value = rnd_gold();
        if (save(VS_MAGIC))
            value += rnd_gold() + rnd_gold() + rnd_gold() + rnd_gold();
        Item *gold = new Gold(value);
        monster->pack.push_front(gold);
    }

    if (print)
    {
        addmsg("you have defeated ");
        if (game->hero().is_blind())
            msg(it);
        else
            msg("the %s", monster->get_name().c_str());
    }

    game->hero().gain_experience(monster->experience());
    
    //Get rid of the monster.
    remove_monster(monster, true);
}
