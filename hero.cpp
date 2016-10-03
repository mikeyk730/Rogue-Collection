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
#include "fight.h"
#include "potions.h"
#include "slime.h"
#include "scrolls.h"
#include "things.h"
#include "sticks.h"

namespace
{
    // Each level is twice the previous
    const long e_levels[20] = { 10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240,
        20480, 40960, 81920, 163840, 327680, 655360, 1310720, 2621440, 0 };
}

Hero::Hero() :
    Agent()
{
    init_player();
}

void Hero::calculate_roll_stats(Agent *defender, Item *object, bool hurl,
    int* hit_plus, std::string* damage_string, int* damage_plus)
{
    if (object == NULL) {
        *damage_string = m_stats.m_damage;
        *damage_plus = 0;
        *hit_plus = 0;
        return;
    }

    //Drain a staff of striking
    //mdk: should this only happen when zapped?  currently happens during melee as well
    Stick* stick = dynamic_cast<Stick*>(object);
    if (stick && stick->m_which == WS_HIT)
    {
        stick->drain_striking();
    }

    *damage_string = object->melee_damage();
    *hit_plus = object->hit_plus();
    *damage_plus = object->damage_plus();

    //vorpally enchanted object adds +4,+4 against target
    Weapon* weapon = dynamic_cast<Weapon*>(object);
    if (weapon && weapon->is_vorpalized_against(dynamic_cast<Monster*>(defender))) {
        *hit_plus += 4;
        *damage_plus += 4;
    }

    Item* current_weapon = get_current_weapon();
    if (object == current_weapon)
    {
        //rings can boost the wielded object
        if (is_ring_on_hand(LEFT, R_ADDDAM))
            *damage_plus += get_ring(LEFT)->get_ring_level();
        else if (is_ring_on_hand(LEFT, R_ADDHIT))
            *hit_plus += get_ring(LEFT)->get_ring_level();

        if (is_ring_on_hand(RIGHT, R_ADDDAM))
            *damage_plus += get_ring(RIGHT)->get_ring_level();
        else if (is_ring_on_hand(RIGHT, R_ADDHIT))
            *hit_plus += get_ring(RIGHT)->get_ring_level();
    }

    if (hurl) {
        //mdk: the original code never used throw damage except for arrows and crossbow bolts.
        //I've decided to use it for weapons that don't require a launcher too.  IS_MISL is
        //still meaningless.
        if (game->options.use_throw_damage() && object->launcher() == NONE) {
            *damage_string = object->throw_damage();
        }
        //if we've used the right object to launch the projectile, we use the throw 
        //damage of the projectile, and get the plusses from the launcher.
        else if (current_weapon && object->launcher() == current_weapon->m_which)
        {
            *damage_string = object->throw_damage();
            *hit_plus += current_weapon->hit_plus();
            *damage_plus += current_weapon->damage_plus();
        }
    }
}

int Hero::calculate_armor() const
{
    int armor = m_stats.m_ac;

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
    return calculate_strength_impl(m_stats.m_str);
}

int Hero::calculate_max_strength() const
{
    return calculate_strength_impl(m_stats.m_max_str);
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

bool Hero::eat()
{
    Item* obj = get_item("eat", FOOD);
    if (obj == nullptr)
        return false;

    //mdk: attempting to eat non-food counts as turn
    if (obj->m_type != FOOD) {
        msg("ugh, you would get ill if you ate that");
        return true;
    }

    if (--obj->m_count < 1) {
        m_pack.remove(obj);
        if (obj == get_current_weapon())
            set_current_weapon(NULL); //todo: this should be done automatically when removing from pack
        delete obj;
    }
    ingest();

    if (obj->m_which == 1) {
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

    return true;
}

void Hero::ingest()
{
    if (food_left < 0)
        food_left = 0;
    if (food_left > (STOMACH_SIZE - 20))
        game->sleep_timer += 2 + rnd(5);
    if ((food_left += HUNGER_TIME - 200 + rnd(400)) > STOMACH_SIZE)
        food_left = STOMACH_SIZE;
    hungry_state = 0;
}

void Hero::digest()
{
    if (food_left <= 0)
    {
        if (food_left-- < -STARVE_TIME)
            death('s');
        //the hero is fainting
        if (game->sleep_timer || rnd(5) != 0)
            return;
        game->sleep_timer += rnd(8) + 4;
        set_running(false);
        game->stop_running();
        game->cancel_repeating_cmd();
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
        if (food_left < MORE_TIME && oldfood >= MORE_TIME) {
            hungry_state = 2;
            msg("you are starting to feel weak");
        }
        else if (food_left < 2 * MORE_TIME && oldfood >= 2 * MORE_TIME) {
            hungry_state = 1;
            msg("you are starting to get hungry");
        }
    }
}

//init_player: Roll up the rogue
void Hero::init_player()
{
    m_stats = { 16, 0, 1, 10, 12, "1d4", 12, 16 };
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
    obj->m_count = rnd(15) + 25;
    obj->set_known();
    add_to_pack(obj, true);

    //And his suit of armor
    Armor* armor = new Armor(RING_MAIL, -1);
    set_current_armor(armor);
    add_to_pack(armor, true);

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

    game->screen().mvaddch(m_position, game->level().get_tile(m_position));
    do {
        rm = rnd_room();
        rnd_pos(rm, &c);
    } while (!(step_ok(game->level().get_tile_or_monster(c)))); //todo:bug: can we teleport onto a xerox?
    if (rm != m_room) {
        leave_room(m_position);
        m_position = c;
        enter_room(m_position);
    }
    else { m_position = c; look(true); }
    game->screen().mvaddch(m_position, PLAYER);
    //turn off IS_HELD in case teleportation was done while fighting a Flytrap
    if (is_held()) {
        set_is_held(false);
    }
    game->bear_trap_turns = 0;
    game->cancel_repeating_cmd();
    game->stop_running();
    clear_typeahead_buffer();

    //Teleportation can be a confusing experience (unless you really are a wizard)
    if (is_confused())
        lengthen(unconfuse, rnd(4) + 2);
    else
        fuse(unconfuse, 0, rnd(4) + 2);
    set_confused(true);
}

//check_level: Check to see if the guy has gone up a level.
void Hero::check_level()
{
    int i, add, olevel;

    for (i = 0; e_levels[i] != 0; i++)
        if (e_levels[i] > experience())
            break;
    i++;
    olevel = m_stats.m_level;
    m_stats.m_level = i;
    if (i > olevel)
    {
        add = roll(i - olevel, 10);
        m_stats.m_max_hp += add;
        increase_hp(add, false, false);
        if (game->options.use_exp_level_names())
            msg("and achieve the rank of \"%s\"", level_titles[i - 1]);
        else
            msg("Welcome to level %d", i);
    }
}

//raise_level: The guy just magically went up a level.
void Hero::raise_level()
{
    m_stats.m_exp = e_levels[m_stats.m_level - 1] + 1L;
    check_level();
}

void Hero::gain_experience(int exp)
{
    Agent::gain_experience(exp);
    check_level();
}

void Hero::reduce_level()
{
    --m_stats.m_level;
    if (m_stats.m_level == 0) {
        m_stats.m_exp = 0;
        m_stats.m_level = 1;
    }
    else {
        m_stats.m_exp = e_levels[m_stats.m_level - 1] + 1;
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

    if (distance(monster->m_position, m_position) >= LAMP_DIST &&
        (monster->m_room != m_room || monster->m_room->is_dark() || monster->m_room->is_maze()))
        return false;

    //If we are seeing the enemy of a vorpally enchanted object for the first time, 
    //give the player a hint as to what that object is good for.
    Item* item = get_current_weapon();
    Weapon* weapon = dynamic_cast<Weapon*>(item);
    if (weapon && weapon->is_vorpalized_against(monster) && !weapon->did_flash())
    {
        weapon->set_flashed();
        msg(flash, weapon->name().c_str(), short_msgs() ? "" : intense);
    }
    return true;
}

//can_see: Returns true if the hero can see a certain coordinate.
int Hero::can_see(Coord p)
{
    if (is_blind())
        return false;
    //if the coordinate is close.
    if (distance(p, m_position) < LAMP_DIST)
        return true;
    //if the coordinate is in the same room as the hero, and the room is lit
    return (m_room == get_room_from_position(p) && !m_room->is_dark());
}

void Hero::do_hit(Item* weapon, int thrown, Monster* monster, const char* name)
{
    bool did_confuse = false;

    if (thrown)
        display_throw_msg(weapon, name, "hits", "hit");
    else
        display_hit_msg(NULL, name);

    if (weapon && weapon->m_type == POTION)
    {
        affect_monster(weapon, monster);
        if (!thrown)
        {
            if (--weapon->m_count == 0) {
                m_pack.remove(weapon);
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
        killed_by_hero(monster, true);
    else if (did_confuse && !is_blind())
        msg("the %s appears confused", name);
}

void Hero::do_miss(Item* weapon, int thrown, Monster* monster, const char* name)
{
    if (thrown)
        display_throw_msg(weapon, name, "misses", "missed");
    else
        display_miss_msg(NULL, name);

    if (monster->can_divide() && rnd(100) > 25)
        slime_split(monster);
}

//fight: The player attacks the monster.
Monster* Hero::fight(Coord monster_pos, Item *weapon, bool thrown)
{
    std::string name;
    //Find the monster we want to fight
    Monster* monster = game->level().monster_at(monster_pos);
    if (!monster)
        return 0;

    //Since we are fighting, things are not quiet so no healing takes place.  Cancel any command counts so player can recover.
    game->cancel_repeating_cmd();
    game->turns_since_heal = 0;

    if (monster->is_disguised())
    {
        bool throws_affect_mimics(game->options.throws_affect_mimics());
        if (thrown && !throws_affect_mimics) {
            return 0;
        }
        //Let him know it was really a mimic (if it was one).
        if (!is_blind())
            msg("wait! That's a %s!", monster->get_name().c_str());
    }

    monster->start_run();

    name = this->is_blind() ? "it" : monster->get_name();

    if (attack(monster, weapon, thrown) || (weapon && weapon->m_type == POTION))
    {
        do_hit(weapon, thrown, monster, name.c_str());
        return monster;
    }

    do_miss(weapon, thrown, monster, name.c_str());
    return 0;
}

Ring* Hero::get_ring(int hand) const
{
    return m_rings[hand];
}

void Hero::set_ring(int hand, Ring* ring)
{
    m_rings[hand] = ring;
}

Item* Hero::get_current_weapon() const
{
    return m_weapon;
}

void Hero::set_current_weapon(Item* item)
{
    m_weapon = item;
}

Armor* Hero::get_current_armor() const
{
    return m_armor;
}

void Hero::set_current_armor(Armor* armor)
{
    if (armor)
        armor->set_known();
    m_armor = armor;
}

int Hero::get_pack_size()
{
    int count = 0;
    for (auto it = m_pack.begin(); it != m_pack.end(); ++it) {
        Item* item = *it;
        count += item->m_group ? 1 : item->m_count;
    }
    return count;
}

//add_to_pack: Pick up an object and add it to the pack.  If the argument is non-null use it as the linked_list pointer instead of getting it off the ground.
void Hero::add_to_pack(Item *obj, bool silent)
{
    bool from_floor = false;
    if (!obj)
    {   
        from_floor = true;
        obj = find_obj(m_position, true);
        if (!obj)
            return;
    }

    if (!add_to_list(&obj, from_floor))
        return;

    //If this was the object of something's desire, that monster will get mad and run at the hero
    //todo where used?
    if (from_floor) {
        for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it) {
            Monster* monster = *it;
            if (monster->m_destination && (*monster->m_destination == obj->m_position))
                monster->m_destination = &m_position;
        }
    }

    if (obj->m_type == AMULET) {
        m_had_amulet = true;
    }

    //Notify the user
    if (!silent)
        msg("%s%s (%c)", noterse("you now have "), obj->inventory_name(*this, true).c_str(), pack_char(obj));
}

bool Hero::add_to_list(Item** obj, bool from_floor)
{
    auto it = m_pack.begin();

    //Link it into the pack.  Search the pack for a object of similar type
    //if there isn't one, stuff it at the beginning, if there is, look for one
    //that is exactly the same and just increment the count if there is.
    //Food is always put at the beginning for ease of access, but it
    //is not ordered so that you can't tell good food from bad.  First check
    //to see if there is something in the same group and if there is then
    //increment the count.
    if ((*obj)->m_group)
    {
        for (auto it = m_pack.begin(); it != m_pack.end(); ++it) {
            Item* op = *it;
            if (op->m_group == (*obj)->m_group)
            {
                //Put it in the pack and notify the user
                op->m_count += (*obj)->m_count;
                if (from_floor) {
                    byte floor = (m_room->is_gone()) ? PASSAGE : FLOOR;
                    game->level().items.remove((*obj));
                    game->screen().mvaddch(m_position, floor);
                    game->level().set_tile(m_position, floor);
                }
                delete *obj;
                *obj = op;
                return true;
            }
        }
    }

    //Check if there is room
    if (get_pack_size() >= MAXPACK - 1) {
        msg("you can't carry anything else");
        return false;
    }

    //Check for and deal with scare monster scrolls
    if (is_scare_monster_scroll(*obj)) {
        if ((*obj)->is_found())
        {
            byte floor = (m_room->is_gone()) ? PASSAGE : FLOOR;
            game->level().items.remove(*obj);
            delete *obj;
            game->screen().mvaddch(m_position, floor);
            game->level().set_tile(m_position, floor);
            msg("the scroll turns to dust%s.", noterse(" as you pick it up"));
            return false;
        }
        else (*obj)->set_found();
    }

    if (from_floor) {
        byte floor = (m_room->is_gone()) ? PASSAGE : FLOOR;
        game->level().items.remove(*obj);
        game->screen().mvaddch(m_position, floor);
        game->level().set_tile(m_position, floor);
    }

    //Search for an object of the same type
    bool found_type = false;
    for (; it != m_pack.end(); ++it) {
        if ((*it)->m_type == (*obj)->m_type) {
            found_type = true;
            break;
        }
    }
    //Put it at the end of the pack since it is a new type
    if (!found_type) {
        ((*obj)->m_type == FOOD) ? m_pack.push_front(*obj) :
            m_pack.push_back(*obj);
        return true;
    }

    //Search for an object which is exactly the same
    bool exact = false;
    for (; it != m_pack.end(); ++it) {
        if ((*it)->m_type != (*obj)->m_type)
            break;
        if ((*it)->m_which == (*obj)->m_which) {
            exact = true;
            break;
        }
    }
    //If we found an exact match.  If it is a potion, food, or a scroll, increase the count, otherwise put it with its clones.
    if (exact && does_item_group((*obj)->m_type))
    {
        (*it)->m_count++;
        delete *obj;
        *obj = (*it);
        return true;
    }

    m_pack.insert(it, *obj);
    return true;
}

//pick_up_gold: Add gold to the pack
void Hero::pick_up_gold(int value)
{
    adjust_purse(value);
    msg("you found %d gold pieces", value);
}

bool Hero::has_amulet()
{
    for (auto i = m_pack.begin(); i != m_pack.end(); ++i) {
        if ((*i)->m_type == AMULET)
            return true;
    }
    return false;
}

//true if player ever had amulet
bool Hero::had_amulet()
{
    return m_had_amulet;
}

int Hero::is_ring_on_hand(int hand, int ring) const
{
    return (get_ring(hand) != NULL && get_ring(hand)->m_which == ring);
}

int Hero::is_wearing_ring(int ring) const
{
    return (is_ring_on_hand(LEFT, ring) || is_ring_on_hand(RIGHT, ring));
}

//wield: Pull out a certain object
bool Hero::wield()
{
    //mdk: trying to wield object while old one is cursed counts as turn
    if (!can_drop(get_current_weapon(), false)) {
        return true;
    }

    Item* obj = get_item("wield", WEAPON);
    if (!obj || obj->m_type == ARMOR || is_in_use(obj))
    {
        if (obj && obj->m_type == ARMOR)
            msg("you can't wield armor");
        return false;
    }

    set_current_weapon(obj);
    ifterse("now wielding %s (%c)", "you are now wielding %s (%c)", obj->inventory_name(*this, true).c_str(), pack_char(obj));
    return true;
}

//get_hand: Which hand is the hero interested in?
int get_hand()
{
    for (;;)
    {
        unsaved_msg("left hand or right hand? ");
        int c = readchar();
        if (c == ESCAPE) {
            return -1;
        }
        if (c == 'l' || c == 'L')
            return LEFT;
        else if (c == 'r' || c == 'R')
            return RIGHT;
        msg("please type L or R");
    }
}

bool Hero::put_on_ring()
{
    Item* item = get_item("put on", RING);
    if (!item) {
        return false;
    }

    //Make certain that it is something that we want to wear
    Ring* obj = dynamic_cast<Ring*>(item);
    if (!obj) {
        msg("you can't put that on your finger");
        return false;
    }
        
    if (is_in_use(obj)) {
        return false;
    }

    //find out which hand to put it on
    int ring = -1;
    if (get_ring(LEFT) == NULL) 
        ring = LEFT;
    if (get_ring(RIGHT) == NULL)
        ring = RIGHT;
    if (get_ring(LEFT) == NULL && get_ring(RIGHT) == NULL) 
        if ((ring = get_hand()) < 0)
            return false;
    
    if (ring < 0) { 
        msg("you already have a ring on each hand");
        return false;
    }

    set_ring(ring, obj);

    //Calculate the effect it has on the poor guy.
    switch (obj->m_which)
    {
    case R_SEEINVIS:
        show_invisible();
        break;
    case R_AGGR:
        aggravate_monsters();
        break;
    }
    
    msg("%swearing %s (%c)", noterse("you are now "), obj->inventory_name(*this, true).c_str(), pack_char(obj));
    return true;
}

bool Hero::remove_ring()
{
    int ring;
    if (get_ring(LEFT) == NULL && get_ring(RIGHT) == NULL) {
        msg("you aren't wearing any rings");
        return false;
    }
    else if (get_ring(LEFT) == NULL)
        ring = RIGHT;
    else if (get_ring(RIGHT) == NULL)
        ring = LEFT;
    else if ((ring = get_hand()) < 0)
        return false;

    Item* obj = get_ring(ring);
    if (obj == NULL) {
        msg("not wearing such a ring");
        return false;
    }

    char packchar = pack_char(obj);
    //mdk: attempting to take off cursed ring counts as turn.
    if (can_drop(obj, true))
        msg("was wearing %s(%c)", obj->inventory_name(*this, true).c_str(), packchar);
    return true;
}

//wear_armor: The player wants to wear something, so let him/her put it on.
bool Hero::wear_armor()
{
    if (get_current_armor()) {
        msg("you are already wearing some%s.", noterse(".  You'll have to take it off first"));
        return false;
    }

    Item *obj = get_item("wear", ARMOR);
    if (!obj) 
        return false;

    //mdk: trying to wear non-armor counts as turn
    Armor* armor = dynamic_cast<Armor*>(obj);
    if (!armor) {
        msg("you can't wear that");
        return true;
    }

    waste_time(); //mdk: putting on armor takes 2 turns

    set_current_armor(armor);
    msg("you are now wearing %s", armor->inventory_name(*this, true).c_str());
    return true;
}

//take_off_armor: Get the armor off of the player's back
bool Hero::take_off_armor()
{
    Item *obj = get_current_armor();
    if (!obj) {
        msg("you aren't wearing any armor");
        return false;
    }

    //mdk: trying to take off cursed armor counts as turn
    if (!can_drop(get_current_armor(), true))
        return true;

    set_current_armor(NULL);
    msg("you used to be wearing %c) %s", pack_char(obj), obj->inventory_name(*this, true).c_str());
    return true;
}

//add_haste: Add a haste to the player
int Hero::add_haste(bool is_temporary)
{
    if (is_fast())
    {
        game->sleep_timer += rnd(8);
        set_running(false);
        extinguish(nohaste);
        set_is_fast(false);
        msg("you faint from exhaustion");
        return false;
    }
    else
    {
        set_is_fast(true);
        if (is_temporary)
            fuse(nohaste, 0, rnd(4) + 10);
        return true;
    }
}

void Hero::set_num_actions(int actions)
{
    m_num_actions = actions;
}

bool Hero::decrement_num_actions()
{
    if (m_num_actions > 0) {
        --m_num_actions;
        return true;
    }
    return false;
}
