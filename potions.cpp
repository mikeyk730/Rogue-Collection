//Function(s) for dealing with potions
//potions.c   1.4 (AI Design) 2/12/84

#include <algorithm>
#include <stdio.h>

#include "rogue.h"
#include "item_class.h"
#include "game_state.h"
#include "daemons.h"
#include "daemon.h"
#include "potions.h"
#include "pack.h"
#include "output_interface.h"
#include "io.h"
#include "list.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "rings.h"
#include "monsters.h"
#include "things.h"
#include "hero.h"
#include "level.h"
#include "monster.h"

#define P_CONFUSE   0
#define P_PARALYZE  1
#define P_POISON    2
#define P_STRENGTH  3
#define P_SEEINVIS  4
#define P_HEALING   5
#define P_MFIND     6
#define P_TFIND     7
#define P_RAISE     8
#define P_XHEAL     9
#define P_HASTE     10
#define P_RESTORE   11
#define P_BLIND     12
#define P_NOP       13

static char *rainbow[] =
{
  "amber",
  "aquamarine",
  "black",
  "blue",
  "brown",
  "clear",
  "crimson",
  "cyan",
  "ecru",
  "gold",
  "green",
  "grey",
  "magenta",
  "orange",
  "pink",
  "plaid",
  "purple",
  "red",
  "silver",
  "tan",
  "tangerine",
  "topaz",
  "turquoise",
  "vermilion",
  "violet",
  "white",
  "yellow"
};

#define NCOLORS (sizeof(rainbow)/sizeof(char *))

PotionInfo::PotionInfo()
{
    m_magic_props =
    {
      {"confusion",          8,   5},
      {"paralysis",         10,   5},
      {"poison",             8,   5},
      {"gain strength",     15, 150},
      {"see invisible",      2, 100},
      {"healing",           15, 130},
      {"monster detection",  6, 130},
      {"magic detection",    6, 105},
      {"raise level",        2, 250},
      {"extra healing",      5, 200},
      {"haste self",         4, 190},
      {"restore strength",  14, 130},
      {"blindness",          4,   5},
      {"thirst quenching",   1,   5}
    };

    int i, j;
    bool used[NCOLORS];

    for (i = 0; i < NCOLORS; i++) used[i] = false;
    for (i = 0; i < MAXPOTIONS; i++)
    {
        do j = rnd(NCOLORS); while (used[j]);
        used[j] = true;
        m_identifier.push_back(rainbow[j]);
        if (i > 0)
            m_magic_props[i].prob += m_magic_props[i - 1].prob;
    }
}

Item* create_potion()
{
    int which = pick_one(game->potions().m_magic_props);
    return new Potion(which);
}

void Potion::quaff_confusion()
{
    discover();
    if (!game->hero().is_confused())
    {
        if (game->hero().is_confused())
            lengthen(unconfuse, rnd(8) + HUH_DURATION);
        else
            fuse(unconfuse, 0, rnd(8) + HUH_DURATION);
        game->hero().set_confused(true);
        msg("wait, what's going on? Huh? What? Who?");
    }
}

void Potion::quaff_paralysis()
{
    discover();
    game->hero().increase_sleep_timer(HOLD_TIME);
    msg("you can't move");
}

void Potion::quaff_poison()
{
    char *sick = "you feel %s sick.";

    discover();
    if (!game->hero().is_wearing_ring(R_SUSTSTR)) {
        game->hero().adjust_strength(-(rnd(3) + 1));
        msg(sick, "very");
    }
    else msg(sick, "momentarily");
}

void Potion::quaff_gain_strength()
{
    discover();
    game->hero().adjust_strength(1);
    msg("you feel stronger. What bulging muscles!");
}

void Potion::quaff_see_invisible()
{
    if (!game->hero().sees_invisible()) {
        fuse(unsee_invisible, 0, SEE_DURATION);
        look(false);
        show_invisible();
    }
    sight();
    msg("this potion tastes like %s juice", game->get_environment("fruit").c_str());
}

void Potion::quaff_healing()
{
    discover();
    game->hero().increase_hp(roll(game->hero().m_stats.m_level, 4), true, false);
    sight();
    msg("you begin to feel better");
}

void Potion::quaff_monster_detection()
{
    fuse(turn_see_wrapper, true, HUH_DURATION);
    if (game->level().monsters.empty())
        msg("you have a strange feeling%s.", noterse(" for a moment"));
    else {
        if (turn_see(false))
            discover();
        msg("");
    }
}

void Potion::quaff_magic_detection()
{
    //Potion of magic detection.  Find everything interesting on the level and show him where they are. 
    //Also give hints as to whether he would want to use the object.
    bool discovered = false;

    for (auto i = game->level().items.begin(); i != game->level().items.end(); ++i)
    {
        Item* item = *i;
        if (item->is_magic())
        {
            discovered = true;
            game->screen().mvaddch(item->m_position, get_magic_char(item));
        }
    }
    for (auto m = game->level().monsters.begin(); m != game->level().monsters.end(); ++m) {
        Agent* monster = *m;
        for (auto i = monster->m_pack.begin(); i != monster->m_pack.end(); ++i)
        {
            Item* item = *i;
            if (item->is_magic())
            {
                discovered = true;
                game->screen().mvaddch(monster->m_position, MAGIC);
            }
        }
    }

    if (discovered) {
        discover();
        msg("You sense the presence of magic.");
    }
    else {
        msg("you have a strange feeling for a moment%s.", noterse(", then it passes"));
    }
}

void Potion::quaff_raise_level()
{
    discover();
    msg("you suddenly feel much more skillful");
    game->hero().raise_level();
}

void Potion::quaff_extra_healing()
{
    discover();
    game->hero().increase_hp(roll(game->hero().m_stats.m_level, 8), true, true);
    sight();
    msg("you begin to feel much better");
}

void Potion::quaff_haste_self()
{
    discover();
    if (game->hero().add_haste(true)) {
        msg("you feel yourself moving much faster");
    }
}

void Potion::quaff_restore_strength()
{
    game->hero().restore_strength();
    msg("%syou feel warm all over", noterse("hey, this tastes great.  It makes "));
}

void Potion::quaff_blindness()
{
    discover();
    if (!game->hero().is_blind())
    {
        game->hero().set_blind(true);
        fuse(sight, 0, SEE_DURATION);
        look(false);
    }
    msg("a cloak of darkness falls around you");
}

void Potion::quaff_thirst_quenching()
{
    msg("this potion tastes extremely dull");
}

void(Potion::*potion_functions[MAXPOTIONS])() = {
  &Potion::quaff_confusion,
  &Potion::quaff_paralysis,
  &Potion::quaff_poison,
  &Potion::quaff_gain_strength,
  &Potion::quaff_see_invisible,
  &Potion::quaff_healing,
  &Potion::quaff_monster_detection,
  &Potion::quaff_magic_detection,
  &Potion::quaff_raise_level,
  &Potion::quaff_extra_healing,
  &Potion::quaff_haste_self,
  &Potion::quaff_restore_strength,
  &Potion::quaff_blindness,
  &Potion::quaff_thirst_quenching
};

//do_quaff: Quaff a potion from the pack
bool do_quaff()
{
    Item* item = get_item("quaff", POTION);
    if (!item) 
        return false;

    //Make certain that it is something that we want to drink
    Potion* potion = dynamic_cast<Potion*>(item);
    if (!potion) {
        //mdk: trying to drink non-potion counts as turn
        msg("yuk! Why would you want to drink that?");
        return true;
    }

    if (potion == game->hero().get_current_weapon()) //todo: make happen when remove from pack
        game->hero().set_current_weapon(NULL);

    //Calculate the effect it has on the poor guy.
    (potion->*potion_functions[potion->m_which])();

    update_status_bar();
    potion->call_it();

    //Throw the item away
    if (potion->m_count > 1)
        potion->m_count--;
    else {
        game->hero().m_pack.remove(potion);
        delete potion;
    }

    return true;
}

//show_invisible: Turn on the ability to see invisible
void show_invisible()
{
    game->hero().set_sees_invisible(true);
    std::for_each(game->level().monsters.begin(), game->level().monsters.end(), [](Monster* monster) {
        if (monster->is_invisible() && game->hero().can_see_monster(monster))
        {
            game->screen().mvaddch(monster->m_position, monster->m_disguise);
        }
    });
}

void turn_see_wrapper(int turn_off)
{
    turn_see(turn_off != 0);
}

//turn_see: Put on or off seeing monsters on this level
bool turn_see(bool turn_off)
{
    bool add_new = false;

    std::for_each(game->level().monsters.begin(), game->level().monsters.end(), [turn_off, &add_new](Monster* monster) {

        bool can_see;
        byte was_there;

        game->screen().move(monster->m_position.y, monster->m_position.x);
        can_see = (game->hero().can_see_monster(monster) || (was_there = game->screen().curch()) == monster->m_type);
        if (turn_off)
        {
            if (!game->hero().can_see_monster(monster) && monster->has_tile_beneath())
                game->screen().addch(monster->tile_beneath());
        }
        else
        {
            if (!can_see) { 
                game->screen().standout(); 
                monster->set_tile_beneath(was_there);
            }
            game->screen().addch(monster->m_type);
            if (!can_see) {
                game->screen().standend();
                add_new++; 
            }
        }

    });

    game->hero().set_detects_others(!turn_off);
    return add_new;
}

//th_effect: Compute the effect of this potion hitting a monster.
void affect_monster(Item *potion, Monster *monster)
{
    msg("the flask shatters.");

    switch (potion->m_which)
    {
    case P_CONFUSE: case P_BLIND:
        monster->set_confused(true);
        msg("the %s appears confused", monster->get_name().c_str());
        break;

    case P_PARALYZE:
        monster->hold();
        break;

    case P_HEALING: case P_XHEAL:
        monster->increase_hp(rnd(8), true, false);
        break;

    case P_RAISE:
        monster->m_stats.m_max_hp += 8;
        monster->increase_hp(8, false, false);
        monster->m_stats.m_level++;
        break;

    case P_HASTE:
        monster->set_is_fast(true);
        break;
    }
}

std::string PotionInfo::get_inventory_name(int which, int count) const
{
    char *pb = prbuf;
    std::string color = get_identifier(which);

    if (count == 1) {
        strcpy(pb, "A potion ");
        pb = &prbuf[9];
    }
    else {
        sprintf(pb, "%d potions ", count);
        pb = &pb[strlen(prbuf)];
    }
    if (is_discovered(which) || game->wizard().reveal_items()) {
        chopmsg(pb, "of %s", "of %s(%s)", get_name(which).c_str(), color.c_str());
    }
    else if (!get_guess(which).empty()) {
        chopmsg(pb, "called %s", "called %s(%s)", get_guess(which).c_str(), color.c_str());
    }
    else if (count == 1)
        sprintf(prbuf, "A%s %s potion", vowelstr(color.c_str()), color.c_str());
    else sprintf(prbuf, "%d %s potions", count, color.c_str());

    return prbuf;
}

std::string PotionInfo::get_inventory_name(const Item * obj) const
{
    return get_inventory_name(obj->m_which, obj->m_count);
}

std::string PotionInfo::get_inventory_name(int which) const
{
    return get_inventory_name(which, 1);
}



Potion::Potion(int which)
    : Item(POTION, which)
{
}

Item * Potion::Clone() const
{
    return new Potion(*this);
}

std::string Potion::Name() const
{
    return "potion";
}

std::string Potion::InventoryName() const
{
    return item_class()->get_inventory_name(this);
}

bool Potion::IsMagic() const
{
    return true;
}

bool Potion::IsEvil() const
{
    return (m_which == P_CONFUSE || m_which == P_PARALYZE || m_which == P_POISON || m_which == P_BLIND);
}

int Potion::Worth() const
{
    int worth = item_class()->get_value(m_which);
    worth *= m_count;
    if (!item_class()->is_discovered(m_which)) 
        worth /= 2;
    return worth;
}
