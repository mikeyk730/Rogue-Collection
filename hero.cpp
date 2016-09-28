#include <string.h>

#include "rogue.h"
#include "game_state.h"
#include "hero.h"
#include "rip.h"
#include "main.h"
#include "misc.h"
#include "pack.h"
#include "armor.h"
#include "weapons.h"
#include "io.h"
#include "thing.h"
#include "rings.h"
#include "agent.h"
#include "food.h"
#include "output_interface.h"
#include "level.h"
#include "rooms.h"
#include "daemon.h"
#include "daemons.h"
#include "mach_dep.h"
#include "monster.h"
#include "room.h"

namespace
{
    // Each level is twice the previous
    const long e_levels[20] = { 10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240,
        20480, 40960, 81920, 163840, 327680, 655360, 1310720, 2621440, 0 };
}

Hero::Hero()
{
    init_player();
}

void Hero::calculate_roll_stats(Agent *defender, Item *weapon, bool hurl,
    int* hit_plus, std::string* damage_string, int* damage_plus)
{
    if (weapon == NULL) {
        *damage_string = stats.damage;
        *damage_plus = 0;
        *hit_plus = 0;
        return;
    }

    *damage_string = weapon->get_damage();
    *hit_plus = weapon->get_hit_plus();
    *damage_plus = weapon->get_damage_plus();

    //vorpally enchanted weapon adds +4,+4 against target
    if (weapon->is_vorpalized_against(dynamic_cast<Monster*>(defender))) {
        *hit_plus += 4;
        *damage_plus += 4;
    }

    Item* current_weapon = get_current_weapon();
    if (weapon == current_weapon)
    {
        //rings can boost the wielded weapon
        if (is_ring_on_hand(LEFT, R_ADDDAM))
            *damage_plus += get_ring(LEFT)->get_ring_level();
        else if (is_ring_on_hand(LEFT, R_ADDHIT))
            *hit_plus += get_ring(LEFT)->get_ring_level();

        if (is_ring_on_hand(RIGHT, R_ADDDAM))
            *damage_plus += get_ring(RIGHT)->get_ring_level();
        else if (is_ring_on_hand(RIGHT, R_ADDHIT))
            *hit_plus += get_ring(RIGHT)->get_ring_level();


        //if we've used the right weapon to launch the projectile, we get benefits
        if (hurl && weapon->is_projectile() && weapon->launcher() == current_weapon->which)
        {
            *damage_string = weapon->get_throw_damage();
            *hit_plus += current_weapon->get_hit_plus();
            *damage_plus += current_weapon->get_damage_plus();
        }
    }

    //Drain a staff of striking
    if (weapon->type == STICK && weapon->which == WS_HIT)
    {
        if (weapon->get_charges() == 0) {
            *damage_string = "0d0";
        }
        weapon->drain_striking();
    }
}

int Hero::calculate_armor() const
{
    int armor = stats.ac;

    if (get_current_armor() != NULL)
        armor = get_current_armor()->get_armor_class();
    if (is_ring_on_hand(LEFT, R_PROTECT))
        armor -= get_ring(LEFT)->get_ring_level();
    if (is_ring_on_hand(RIGHT, R_PROTECT))
        armor -= get_ring(RIGHT)->get_ring_level();

    return armor;
}

int Hero::calculate_strength() const
{
    return calculate_strength_impl(stats.m_str);
}

int Hero::calculate_max_strength() const
{
    return calculate_strength_impl(stats.m_max_str);
}

int Hero::calculate_strength_impl(int strength) const
{
    if (is_ring_on_hand(LEFT, R_ADDSTR))
        strength += get_ring(LEFT)->get_ring_level();
    if (is_ring_on_hand(RIGHT, R_ADDSTR))
        strength += get_ring(RIGHT)->get_ring_level();
    return (strength > 31) ? 31 : strength;
}

std::string Hero::get_name()
{
    return m_name;
}

void Hero::set_name(const std::string& name)
{
    m_name = name;
}

int Hero::get_purse()
{
  return m_purse;
}

void Hero::adjust_purse(int delta)
{
  m_purse += delta;
  if (m_purse < 0) 
    m_purse = 0;
}

void Hero::toggle_wizard()
{
    m_wizard = !m_wizard;
    if (m_wizard)
        m_cheated = true;
    msg(is_wizard() ? "You are now a wizard!" : "You feel your magic powers fade away");
}

bool Hero::is_wizard() const
{
  return m_wizard;
}

bool Hero::did_cheat() const
{
  return m_cheated;
}

void Hero::eat(Item* obj)
{
    if (obj->type != FOOD) {
        msg("ugh, you would get ill if you ate that");
        return;
    }

    if (--obj->count < 1) {
        pack.remove(obj);
        if (obj == get_current_weapon())
            set_current_weapon(NULL);
        delete obj;
    }
    ingest();

    if (obj->which == 1) {
        msg("my, that was a yummy %s", game->get_environment("fruit").c_str());
    }
    else if (rnd(100) > 70) {
        msg("yuk, this food tastes awful");
        gain_experience(1);
    }
    else {
        msg("yum, that tasted good");
    }

    if (game->sleep_timer)
        msg("You feel bloated and fall asleep");
}

void Hero::ingest()
{
  if (food_left<0)
    food_left = 0;
  if (food_left>(STOMACH_SIZE-20))
    game->sleep_timer += 2+rnd(5);
  if ((food_left += HUNGER_TIME-200+rnd(400))>STOMACH_SIZE)
    food_left = STOMACH_SIZE;
  hungry_state = 0;
}

void Hero::digest()
{
  if (food_left<=0)
  {
    if (food_left--<-STARVE_TIME) 
        death('s');
    //the hero is fainting
    if (game->sleep_timer || rnd(5)!=0)
        return;
    game->sleep_timer += rnd(8)+4;
    set_running(false);
    game->modifiers.m_running = false;
    repeat_cmd_count = 0;
    hungry_state = 3;
    msg("%syou faint from lack of food", noterse("you feel very weak. "));
  }
  else
  {
    int oldfood = food_left;
    //If you are in 40 column mode use food twice as fast (e.g. 3-(80/40) = 1, 3-(40/40) = 2 : pretty gross huh?)
    int deltafood = 1;
    if (!game->wizard().no_ring_hunger()) {
        deltafood += ring_eat(LEFT);
        deltafood += ring_eat(RIGHT);
    }
    if (in_small_screen_mode())
        deltafood *= 2;
    if (!game->wizard().no_hunger()) {
        food_left -= deltafood;
    }
    if (food_left<MORE_TIME && oldfood>=MORE_TIME) {
        hungry_state = 2;
        msg("you are starting to feel weak");
    }
    else if (food_left<2*MORE_TIME && oldfood>=2*MORE_TIME) {
        hungry_state = 1;
        msg("you are starting to get hungry");
    }
  }
}

//init_player: Roll up the rogue
void Hero::init_player()
{
  stats = { 16, 0, 1, 10, 12, "1d4", 12, 16 };
  food_left = HUNGER_TIME;

  //Give the rogue his weaponry.  First a mace.
  Item *obj;
  obj = new Weapon(MACE, 1, 1);
  obj->set_known();
  add_to_pack(obj, true);
  set_current_weapon(obj);

  //Now a +1 bow
  obj = new Weapon(BOW, 1, 0);
  obj->set_known();
  add_to_pack(obj, true);

  //Now some arrows
  obj = new Weapon(ARROW, 0, 0);
  obj->count = rnd(15)+25;
  obj->set_known();
  add_to_pack(obj, true);

  //And his suit of armor
  obj = new Armor(RING_MAIL, -1);
  obj->set_known();
  set_current_armor(obj);
  add_to_pack(obj, true);

  //Give him some food too
  obj = new Food(0);
  add_to_pack(obj, true);
}

int Hero::get_hungry_state()
{
  return hungry_state;
}

int Hero::get_food_left()
{
    return food_left;
}

//teleport: Bamf the hero someplace else
void Hero::teleport()
{
    struct Room* rm;
    Coord c;

    game->screen().mvaddch(pos, game->level().get_tile(pos));
    do {
        rm = rnd_room();
        rnd_pos(rm, &c);
    } while (!(step_ok(game->level().get_tile_or_monster(c))));
    if (rm != room) {
        leave_room(&pos);
        pos = c;
        enter_room(&pos);
    }
    else { pos = c; look(true); }
    game->screen().mvaddch(pos, PLAYER);
    //turn off IS_HELD in case teleportation was done while fighting a Flytrap
    if (is_held()) {
        set_is_held(false);
    }
    game->no_move = 0;
    repeat_cmd_count = 0;
    game->modifiers.m_running = false;
    clear_typeahead_buffer();
    //Teleportation can be a confusing experience (unless you really are a wizard)
    if (!is_wizard())
    {
        if (is_confused())
            lengthen(unconfuse, rnd(4) + 2);
        else
            fuse(unconfuse, 0, rnd(4) + 2);
        set_confused(true);
    }
}

//check_level: Check to see if the guy has gone up a level.
void Hero::check_level()
{
    int i, add, olevel;

    for (i = 0; e_levels[i] != 0; i++) 
        if (e_levels[i] > experience()) 
            break;
    i++;
    olevel = stats.level;
    stats.level = i;
    if (i > olevel)
    {
        add = roll(i - olevel, 10);
        stats.max_hp += add;
        increase_hp(add, false, false);
        if (game->use_level_names())
            msg("and achieve the rank of \"%s\"", level_titles[i - 1]);
        else
            msg("Welcome to level %d", i);
    }
}

//raise_level: The guy just magically went up a level.
void Hero::raise_level()
{
    stats.m_exp = e_levels[stats.level - 1] + 1L;
    check_level();
}

void Hero::gain_experience(int exp)
{
    Agent::gain_experience(exp);
    check_level();
}

void Hero::reduce_level()
{
    --stats.level;
    if (stats.level == 0) {
        stats.m_exp = 0;
        stats.level = 1;
    }
    else {
        stats.m_exp = e_levels[stats.level - 1] + 1;
    }
}

//can_see_monster: Return true if the hero can see the monster
bool Hero::can_see_monster(Monster *monster)
{
    // player is blind
    if (is_blind())
        return false;

    //monster is invisible, and can't see invisible
    if (monster->is_invisible() && !sees_invisible())
        return false;

    if (distance(monster->pos, pos) >= LAMP_DIST &&
        (monster->room != room || monster->room->is_dark() || monster->room->is_maze()))
        return false;

    //If we are seeing the enemy of a vorpally enchanted weapon for the first time, 
    //give the player a hint as to what that weapon is good for.
    Item* weapon = get_current_weapon();
    if (weapon && weapon->is_vorpalized_against(monster) && !weapon->did_flash())
    {
        weapon->set_flashed();
        msg(flash, get_weapon_name(weapon->which), short_msgs() ? "" : intense);
    }
    return true;
}

//can_see: Returns true if the hero can see a certain coordinate.
int Hero::can_see(Coord p)
{
    if (is_blind())
        return false;
    //if the coordinate is close.
    if (distance(p, pos) < LAMP_DIST)
        return true;
    //if the coordinate is in the same room as the hero, and the room is lit
    return (room == get_room_from_position(&p) && !room->is_dark());
}
