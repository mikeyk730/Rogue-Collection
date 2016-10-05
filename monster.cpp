#include <sstream>
#include "monster.h"
#include "main.h"
#include "monsters.h"
#include "item.h"
#include "game_state.h"
#include "hero.h"
#include "room.h"
#include "rooms.h"
#include "level.h"
#include "misc.h"
#include "scrolls.h"
#include "sticks.h"
#include "output_interface.h"
#include "move.h"
#include "fight.h"
#include "io.h"
#include "rip.h"
#include "mach_dep.h"
#include "rings.h"
#include "armor.h"
#include "gold.h"

#define DRAGONSHOT  5 //one chance in DRAGONSHOT that a dragon will flame

 
bool Monster::can_divide() const
{
    return (m_ex_flags & EX_DIVIDES) != 0;
}

bool Monster::is_stationary() const {
    return (m_ex_flags & EX_STATIONARY) != 0;
}

bool Monster::can_hold() const {
    return (m_ex_flags & EX_HOLDS) != 0;
}

bool Monster::increases_dmg() const {
    return (m_ex_flags & EX_INCREASE_DMG) != 0;
}

bool Monster::shoots_fire() const {
    return (m_ex_flags & EX_SHOOTS_FIRE) != 0;
}

bool Monster::immune_to_fire() const {
    return (m_ex_flags & EX_FIRE_IMMUNITY) != 0;
}

bool Monster::shoots_ice() const {
    return (m_ex_flags & EX_SHOOTS_ICE) != 0;
}

bool Monster::unfreezes_player() const
{
    return (m_ex_flags & EX_UNFREEZES) != 0;
}

bool Monster::freezes_player() const
{
    return (m_ex_flags & EX_FREEZES) != 0;
}

bool Monster::causes_confusion() const {
    return (m_ex_flags & EX_CONFUSES) != 0;
}

bool Monster::is_mimic() const {
    return (m_ex_flags & EX_MIMICS) != 0;
}

bool Monster::is_disguised() const {
    return is_mimic() && m_type != m_disguise;
}

bool Monster::drops_gold() const {
    return (m_ex_flags & EX_DROPS_GOLD) != 0;
}

bool Monster::steals_gold() const {
    return (m_ex_flags & EX_STEALS_GOLD) != 0;
}

bool Monster::steals_magic() const {
    return (m_ex_flags & EX_STEALS_MAGIC) != 0;
}

bool Monster::drains_life() const {
    return (m_ex_flags & EX_DRAINS_MAXHP) != 0;
}

bool Monster::drains_exp() const {
    return (m_ex_flags & EX_DRAINS_EXP) != 0;
}

bool Monster::drains_strength() const {
    return (m_ex_flags & EX_DRAINS_STR) != 0;
}

bool Monster::rusts_armor() const {
    return (m_ex_flags & EX_RUSTS_ARMOR) != 0;
}

bool Monster::dies_from_attack() const {
    return (m_ex_flags & EX_SUICIDES) != 0;
}

bool Monster::guards_gold() const
{
    return (m_ex_flags & EX_GUARDS_GOLD) != 0;
}

bool Monster::slow_when_far() const
{
    return (m_ex_flags & EX_SLOW_WHEN_FAR) != 0;
}

bool Monster::no_fight_msg() const
{
    return (m_ex_flags & EX_NO_FIGHT_MSG) != 0;
}

std::string Monster::get_name()
{
    return get_monster_name(m_type);
}

bool Monster::is_monster_confused_this_turn() const {
    return ((is_confused() && rnd(5) != 0) ||
        // Phantoms are slightly confused all of the time, and bats are quite confused all the time
        m_confused_chance && rnd(m_confused_chance) == 0);
}

void Monster::reveal_disguise() {
    m_disguise = m_type;
}

void Monster::set_dirty(bool enable) {
    m_dirty = enable ? 1 : 0;
}

bool Monster::is_dirty() {
    return m_dirty == 1;
}

void Monster::hold()
{
    set_running(false);
    set_is_held(true);
}

//start_run: Set a monster running after something
void Monster::start_run(bool obtain)
{
    //Start the beastie running
    set_running(true);
    set_is_held(false);
    if (obtain)
        obtain_target();
}

void Monster::start_run(Agent* a)
{
    start_run(false);
    set_destination(a);
}

bool Monster::is_going_to(Coord c)
{
    return m_destination && *m_destination == c;
}

void Monster::set_destination(Coord * d)
{
    m_destination = d;

    std::ostringstream ss;
    ss << get_name() << " " << position() << " setting target " << *d;
    game->log("monster", ss.str());

}

bool Monster::has_tile_beneath() const
{
    return m_tile_beneath != UNSET;
}

byte Monster::tile_beneath() const
{
    return m_tile_beneath;
}

void Monster::set_tile_beneath(byte c)
{
    m_tile_beneath = c;
}

void Monster::reload_tile_beneath()
{
    Coord p = position();
    m_tile_beneath = game->level().get_tile(p);
}

void Monster::render()
{
    game->screen().mvaddch(position(), m_disguise);
}

void Monster::invalidate_tile_beneath()
{
    m_tile_beneath = UNSET;
}

//give_pack: Give a pack to a monster if it deserves one
void Monster::give_pack()
{
    if (rnd(100) < get_carry_probability())
        m_pack.push_front(Item::CreateItem());
}

void Monster::set_destination(Agent * a)
{
    a->set_as_target_of(this);

    std::ostringstream ss;
    ss << get_name() << " " << position() << " setting target " << a->position() << " " << a->get_name();
    game->log("monster", ss.str());
}

void Monster::set_destination(Item * i)
{
    i->set_as_target_of(this);

    std::ostringstream ss;
    ss << get_name() << " " << position() << " setting target " << i->position() << " " << i->name();
    game->log("monster", ss.str());

}

//do_chase: Make one thing chase another.
Monster* Monster::do_chase() //todo: understand
{
    //moving monsters tell no lies
    //mdk:bugfix: originally some actions (aggravate monsters, drain life) could cause
    //a mimic to start running while still disguised
    reveal_disguise();

    //If gold has been taken, target the hero
    if (is_greedy() && room()->m_gold_val == 0) {
        set_destination(&game->hero());
    }

    //Find room of the target
    Room* destination_room = game->hero().room();
    if (*m_destination != game->hero().position())
        destination_room = game->level().get_room_from_position(*m_destination);
    if (destination_room == NULL)
        return 0;


    int mindist = 32767, dist;
    Coord tempdest; //Temporary destination for chaser


    Room* monster_room = room(); //Find room of chaser
                                     //We don't count doors as inside rooms for this routine
    bool door = game->level().get_tile(position()) == DOOR;


    bool repeat;
    do {
        repeat = false;
        //If the object of our desire is in a different room, and we are not in a maze,
        //run to the door nearest to our goal.
        if (monster_room != destination_room && (monster_room->is_maze()) == 0)
        {
            //loop through doors
            for (int i = 0; i < monster_room->m_num_exits; i++)
            {
                dist = distance(*m_destination, monster_room->m_exits[i]);
                if (dist < mindist) {
                    tempdest = monster_room->m_exits[i];
                    mindist = dist;
                }
            }
            if (door)
            {
                monster_room = game->level().get_passage(position());
                door = false;
                repeat = true;
                continue;
            }
        }
        else
        {
            tempdest = *m_destination;
            //For monsters which can fire bolts at the poor hero, we check to see if 
            // (a) the hero is on a straight line from it, and 
            // (b) that it is within shooting distance, but outside of striking range.
            if ((shoots_fire() || shoots_ice()) &&
                (position().y == game->hero().position().y || position().x == game->hero().position().x || abs(position().y - game->hero().position().y) == abs(position().x - game->hero().position().x)) &&
                ((dist = distance(position(), game->hero().position())) > 2 && dist <= BOLT_LENGTH*BOLT_LENGTH) && !powers_cancelled() && rnd(DRAGONSHOT) == 0)
            {
                game->stop_run_cmd();
                Coord delta;
                delta.y = sign(game->hero().position().y - position().y);
                delta.x = sign(game->hero().position().x - position().x);
                return fire_bolt(position(), &delta, shoots_fire() ? "flame" : "frost");
            }
        }

    } while (repeat);

    //This now contains what we want to run to this time so we run to it. If we hit it we either want to fight it or stop running
    Coord next_position;
    chase(&tempdest, &next_position);

    if (next_position == game->hero().position()) {
        return attack_player();
    }
    else if (next_position == *m_destination)
    {
        for (auto it = game->level().items.begin(); it != game->level().items.end(); ) {
            Item* obj = *(it++);
            if (is_going_to(obj->position()))
            {
                std::ostringstream ss;
                ss << get_name() << " " << position() << " reached target " << obj->position();
                game->log("monster", ss.str());

                //mdk: it looks like the intent of the original code was for orcs to
                //pick up gold and then chase the player, instead of just guarding it.
                //I'm keeping the original behavior for now.
                if (guards_gold() && dynamic_cast<Gold*>(obj))
                    break;

                ss.str(std::string());
                ss << get_name() << " " << position() << " obtaining " << obj->name();
                game->log("monster", ss.str());

                byte oldchar;
                game->level().items.remove(obj);
                m_pack.push_front(obj);
                oldchar = (room()->is_gone()) ? PASSAGE : FLOOR;
                game->level().set_tile(obj->position(), oldchar);
                if (game->hero().can_see(obj->position()))
                    game->screen().mvaddch(obj->position(), oldchar);
                obtain_target();
                break;
            }
        }
    }

    if (is_stationary())
        return 0;
    //If the chasing thing moved, update the screen
    do_screen_update(next_position);
    return 0;
}

void Monster::do_screen_update(Coord next_position)
{
    if (has_tile_beneath())
    {
        if (tile_beneath() == ' ' && game->hero().can_see(position()) && game->level().get_tile(position()) == FLOOR)
            game->screen().mvaddch(position(), (char)FLOOR);
        else if (tile_beneath() == FLOOR && !game->hero().can_see(position()) && !game->hero().detects_others())
            game->screen().mvaddch(position(), ' ');
        else
            game->screen().mvaddch(position(), tile_beneath());
    }

    Room *orig_room = room();
    if (!equal(next_position, position()))
    {
        Room* next_room = game->level().get_room_from_position(next_position);
        if (!next_room) {
            return;
        }
        set_room(next_room);
        if (orig_room != room())
            obtain_target();
        set_position(next_position);
    }

    if (game->hero().can_see_monster(this))
    {
        if (game->level().is_passage(next_position))
            game->screen().standout();
        //mdk:tile is fetched from screen so detected monster doesn't reveal level 
        set_tile_beneath(game->screen().mvinch(next_position));
        game->screen().mvaddch(next_position, m_disguise);
    }
    else if (game->hero().detects_others())
    {
        game->screen().standout();
        set_tile_beneath(game->screen().mvinch(next_position));
        game->screen().mvaddch(next_position, m_type);
    }
    else
        invalidate_tile_beneath();

    if (tile_beneath() == FLOOR && orig_room->is_dark())
        set_tile_beneath(' ');

    game->screen().standend();
}

//chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns true if we want to keep on chasing later. false if we reach the goal.
void Monster::chase(Coord *chasee_pos, Coord* next_position)
{
    int x, y;
    int dist, thisdist;
    Coord chaser_pos;
    byte ch;
    int plcnt = 1;

    chaser_pos = position();
    //If the thing is confused, let it move randomly. Phantoms are slightly confused all of the time, and bats are quite confused all the time
    if (is_monster_confused_this_turn())
    {
        //get a valid random move
        rndmove(this, next_position);
        dist = distance(*next_position, *chasee_pos);
        //Small chance that it will become un-confused
        if (rnd(30) == 17)
            set_confused(false);
    }
    //Otherwise, find the empty spot next to the chaser that is closest to the chasee.
    else
    {
        int ey, ex;

        //This will eventually hold where we move to get closer. If we can't find an empty spot, we stay where we are.
        dist = distance(chaser_pos, *chasee_pos);
        *next_position = chaser_pos;
        ey = chaser_pos.y + 1;
        ex = chaser_pos.x + 1;
        for (x = chaser_pos.x - 1; x <= ex; x++)
        {
            for (y = chaser_pos.y - 1; y <= ey; y++)
            {
                Coord try_pos;

                try_pos.x = x;
                try_pos.y = y;
                if (offmap({ x,y }) || !diag_ok(chaser_pos, try_pos)) continue;
                ch = game->level().get_tile_or_monster({ x,y }); //todo:bug: can chaser step on mimic?
                if (step_ok(ch))
                {
                    //If it is a scroll, it might be a scare monster scroll so we need to look it up to see what type it is.
                    if (ch == SCROLL)
                    {
                        Item *obj = 0;
                        for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it)
                        {
                            obj = *it;
                            if (equal(try_pos, obj->position()))
                                break;
                            obj = 0;
                        }
                        if (is_scare_monster_scroll(obj))
                            continue;
                    }
                    //If we didn't find any scrolls at this place or it wasn't a scare scroll, then this place counts
                    thisdist = distance({ x, y }, *chasee_pos);
                    if (thisdist < dist) { plcnt = 1; *next_position = try_pos; dist = thisdist; }
                    else if (thisdist == dist && rnd(++plcnt) == 0) { *next_position = try_pos; dist = thisdist; }
                }
            }
        }
    }
}

//obtain_target: find the proper destination for the monster
void Monster::obtain_target()
{
    // if we're in the same room as the player, or can see the player, then we go after the player.
    // if we have a chance to carry an item, we may go after an unclaimed item in the same room.
    int carry_prob;
    if ((carry_prob = get_carry_probability()) <= 0 || in_same_room_as(&game->hero()) || game->hero().can_see_monster(this)) {
        set_destination(&game->hero());
        return;
    }

    for (auto i = game->level().items.begin(); i != game->level().items.end(); ++i)
    {
        Item* obj = *i;
        if (is_scare_monster_scroll(obj))
            continue;

        if (in_same_room_as(obj) && rnd(100) < carry_prob)
        {
            // don't go after the same object as another monster
            Agent* monster = 0;
            for (auto m = game->level().monsters.begin(); m != game->level().monsters.end(); ++m) {
                if ((*m)->is_going_to(obj->position())) {
                    monster = *m;
                    break;
                }
            }
            if (monster == NULL) {
                set_destination(obj);
                return;
            }
        }
    }
    set_destination(&game->hero());
}

void increase_damage_stats(Monster* mp)
{
    std::ostringstream ss;
    ss << ++(mp->m_flytrap_count) << "d1";
    mp->m_stats.m_damage = ss.str();
}

void thaw_player()
{
    //When an Ice Monster hits you, you get unfrozen faster
    if (game->hero().get_sleep_turns() > 1)
        game->hero().decrement_sleep_turns();
}


bool Monster::rust_attack()
{
    //If a rust monster hits, you lose armor, unless that armor is leather or there is a magic ring
    if (game->hero().get_current_armor() && game->hero().get_current_armor()->get_armor_class() < 9 && game->hero().get_current_armor()->m_which != LEATHER)
        if (game->hero().is_wearing_ring(R_SUSTARM))
            msg("the rust vanishes instantly");
        else {
            msg("your armor weakens, oh my!");
            game->hero().get_current_armor()->weaken_armor();
            return true;
        }
        return false;
}

bool Monster::freeze_attack()
{
    //mdk: i've had to guess on the implementation based on playing the original
    if (!game->hero().get_sleep_turns()) {
        msg("You are frozen by the %s", get_name().c_str());
        game->hero().increase_sleep_turns(rnd(20)+1);
        return true;
    }
    return false;
}

bool Monster::drain_strength_attack()
{
    //Rattlesnakes have poisonous bites
    if (!save(VS_POISON))
        if (!game->hero().is_wearing_ring(R_SUSTSTR)) {
            game->hero().adjust_strength(-1);
            msg("you feel a bite in your leg%s", noterse(" and now feel weaker"));
            return true;
        }
        else
            msg("a bite momentarily weakens you");

    return false;
}

// return true if attack succeeded
bool Monster::steal_gold_attack()
{
    //Leprechaun steals some gold
    long lastpurse;

    lastpurse = game->hero().get_purse();
    game->hero().adjust_purse(-rnd_gold());
    if (!save(VS_MAGIC))
        game->hero().adjust_purse(-(rnd_gold() + rnd_gold() + rnd_gold() + rnd_gold()));
    if (game->hero().get_purse() != lastpurse)
        msg("your purse feels lighter");

    return true;
}

bool Monster::steal_item_attack()
{
    const char *she_stole = "she stole %s!";

    //Nymphs steal a magic item, look through the pack and pick out one we like.
    Item* item = NULL;
    int nobj = 0;
    for (auto it = game->hero().m_pack.begin(); it != game->hero().m_pack.end(); ++it) {
        Item* obj = *it;
        if (obj != game->hero().get_current_armor() && obj != game->hero().get_current_weapon() &&
            obj != game->hero().get_ring(LEFT) && obj != game->hero().get_ring(RIGHT) &&
            obj->is_magic() && rnd(++nobj) == 0)
            item = obj;
    }
    if (item == NULL)
        return false;

    if (item->m_count > 1 && item->m_group == 0)
    {
        int oc;
        oc = item->m_count--;
        item->m_count = 1;
        msg(she_stole, item->inventory_name(game->hero(), true).c_str());
        item->m_count = oc;
    }
    else {
        game->hero().m_pack.remove(item);
        msg(she_stole, item->inventory_name(game->hero(), true).c_str());
        delete item;
    }
    return true;
}

//common function for vampires and wraiths
bool vampire_wraith_attack(Monster* monster)
{
    //Wraiths might drain energy levels, and Vampires can steal max hp
    if (rnd(100) < (monster->drains_exp() ? 15 : 30)) // vampires are twice as likely to connect
    {
        int damage;

        if (monster->drains_exp())
        {
            if (game->hero().experience() == 0)
                death(monster->m_type); //All levels gone
            game->hero().reduce_level();
            damage = roll(1, 10);
        }
        else
            damage = roll(1, 5); //vampires only half as strong

        game->hero().m_stats.m_max_hp -= damage;
        if (game->hero().m_stats.m_max_hp < 1)
            death(monster->m_type);
        game->hero().decrease_hp(damage, false);

        msg("you suddenly feel weaker");
        return true;
    }
    return false;
}

bool Monster::drain_life_attack()
{
    return vampire_wraith_attack(this);
}

bool Monster::drain_exp_attack()
{
    return vampire_wraith_attack(this);
}

bool Monster::hold_attack()
{
    game->hero().set_held_by(this);  //Flytrap stops the poor guy from moving
    return true;
}



//attack: The monster attacks the player
Monster* Monster::attack_player()
{
    std::string name;
    bool attack_success = false;

    //Since this is an attack, stop running and any healing that was going on at the time.
    game->stop_run_cmd();
    game->cancel_repeating_cmd();
    game->turns_since_heal = 0;

    if (is_disguised() && !game->hero().is_blind())
        m_disguise = m_type;
    name = game->hero().is_blind() ? "it" : get_name();

    if (attack(&game->hero(), NULL, false))
    {
        if (!no_fight_msg()) {
            display_hit_msg(name.c_str(), NULL);
        }
        if (game->hero().get_hp() <= 0)
            death(m_type); //Bye bye life ...

        //todo: modify code, so enemy can have more than one power
        if (!powers_cancelled()) {
            if (increases_dmg()) {
                increase_damage_stats(this);
            }
            if (unfreezes_player()) {
                thaw_player();
            }

            //special attacks
            if (can_hold()) {
                attack_success = hold_attack();
            }
            else if (freezes_player())
            {
                attack_success = freeze_attack();
            }
            else if (rusts_armor())
            {
                attack_success = rust_attack();
            }
            else if (steals_gold())
            {
                attack_success = steal_gold_attack();
            }
            else if (steals_magic())
            {
                attack_success = steal_item_attack();
            }
            else if (drains_strength())
            {
                attack_success = drain_strength_attack();
            }
            else if (drains_life())
            {
                attack_success = drain_life_attack();
            }
            else if (drains_exp())
            {
                attack_success = drain_exp_attack();
            }

            if (attack_success && dies_from_attack())
            {
                remove_monster(this, false);
                return this;
            }
        }
    }
    else
    {
        if (increases_dmg())
        {
            if (!game->hero().decrease_hp(m_flytrap_count, true))
                death(m_type); //Bye bye life ...
        }

        // mdk:bugfix: Originally there were no messages when the ice monster missed.
        // I think this goes back to v1.1 when ice monsters didn't have a regular 
        // attack
        if (!no_fight_msg() && !(shoots_ice() && !game->options.ice_monster_miss_bugfix())) {
            display_miss_msg(name.c_str(), NULL);
        }
    }

    clear_typeahead_buffer();
    game->cancel_repeating_cmd();
    update_status_bar();

    return 0;
}
