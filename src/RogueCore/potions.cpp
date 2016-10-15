//Function(s) for dealing with potions
//potions.c   1.4 (AI Design) 2/12/84
#include <sstream>
#include <algorithm>
#include <stdio.h>

#include "random.h"
#include "item_category.h"
#include "game_state.h"
#include "daemons.h"
#include "daemon.h"
#include "potions.h"
#include "pack.h"
#include "output_shim.h"
#include "io.h"
#include "list.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "ring.h"
#include "monsters.h"
#include "things.h"
#include "hero.h"
#include "level.h"
#include "monster.h"

#define HOLD_TIME    spread(2)

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
    potion->Quaff();

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
            game->screen().add_tile(monster->position(), monster->m_disguise);
        }
    });
}

void disable_detect_monsters(int disable)
{
    detect_monsters(!disable);
}

//detect_monsters: Put on or off seeing monsters on this level
bool detect_monsters(bool enable)
{
    game->hero().set_detects_others(enable);
    return game->level().detect_monsters(enable);
}

//th_effect: Compute the effect of this potion hitting a monster.
void affect_monster(Potion *potion, Monster *monster)
{
    msg("the flask shatters.");
    potion->AffectMonster(monster);
}

Potion::Potion()
    : Item(POTION, 0)
{
}

std::string Potion::TypeName() const
{
    return "potion";
}

std::string Potion::InventoryName() const
{
    std::ostringstream ss;

    ItemCategory& info = *Category();

    bool show_true_name(info.is_discovered() || game->wizard().reveal_items());
    bool has_guess(!info.guess().empty());
    
    const std::string& color(info.identifier());

    if (show_true_name || has_guess) {
        if (m_count == 1) {
            ss << "A potion ";
        }
        else {
            ss << m_count << " potions ";
        }

        if (show_true_name) {
            ss << "of " << info.name();
            if (!short_msgs())
                ss << "(" << color << ")";
        }
        else {
            ss << "called " << info.guess();
            if (!short_msgs())
                ss << "(" << color << ")";
        }
    }
    else if (m_count == 1)
        ss << "A" << vowelstr(color.c_str()) << " " << color << " potion";
    else
        ss << m_count << " " << color << " potions";

    return ss.str();
}

bool Potion::IsMagic() const
{
    return true;
}

bool Potion::IsEvil() const
{
    return false;
}

int Potion::Worth() const
{
    int worth = Category()->worth();
    worth *= m_count;
    if (!Category()->is_discovered()) 
        worth /= 2;
    return worth;
}

void Potion::AffectMonster(Monster * m)
{
}

void Confusion::Quaff()
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

void Confusion::AffectMonster(Monster* monster)
{
    monster->set_confused(true);
    msg("the %s appears confused", monster->get_name().c_str());
}

bool Confusion::IsEvil() const
{
    return true;
}

void Paralysis::Quaff()
{
    discover();
    game->hero().increase_sleep_turns(HOLD_TIME);
    msg("you can't move");
}

void Paralysis::AffectMonster(Monster* monster)
{
    monster->hold();
}

bool Paralysis::IsEvil() const
{
    return true;
}

void ThirstQuenching::Quaff()
{
    msg("this potion tastes extremely dull");
}

void Blindness::Quaff()
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

void Blindness::AffectMonster(Monster* monster)
{
    monster->set_confused(true);
    msg("the %s appears confused", monster->get_name().c_str());
}

bool Blindness::IsEvil() const
{
    return true;
}

void RestoreStrength::Quaff()
{
    game->hero().restore_strength();
    msg("%syou feel warm all over", noterse("hey, this tastes great.  It makes "));
}

void HasteSelf::Quaff()
{
    discover();
    if (game->hero().add_haste(true)) {
        msg("you feel yourself moving much faster");
    }
}

void HasteSelf::AffectMonster(Monster* monster)
{
    monster->set_is_fast(true);
}

void ExtraHealing::Quaff()
{
    discover();
    game->hero().increase_hp(roll(game->hero().m_stats.m_level, 8), true, true);
    sight();
    msg("you begin to feel much better");
}

void ExtraHealing::AffectMonster(Monster* monster)
{
    monster->increase_hp(rnd(8), true, false);
}

void RaiseLevel::Quaff()
{
    discover();
    msg("you suddenly feel much more skillful");
    game->hero().raise_level(true);
}

void RaiseLevel::AffectMonster(Monster* monster)
{
    monster->m_stats.m_max_hp += 8;
    monster->increase_hp(8, false, false);
    monster->m_stats.m_level++;
}

void MagicDetection::Quaff()
{
    //Potion of magic detection.  Find everything interesting on the level and show him where they are. 
    //Also give hints as to whether he would want to use the object.
    if (game->level().reveal_magic()) {
        discover();
        msg("You sense the presence of magic.");
    }
    else {
        msg("you have a strange feeling for a moment%s.", noterse(", then it passes"));
    }
}

void MonsterDetection::Quaff()
{
    if (!game->level().has_monsters())
        msg("you have a strange feeling%s.", noterse(" for a moment"));
    else {
        if (detect_monsters(true))
            discover();
        fuse(disable_detect_monsters, true, HUH_DURATION);
        msg("");
    }
}

void Healing::Quaff()
{
    discover();
    game->hero().increase_hp(roll(game->hero().m_stats.m_level, 4), true, false);
    sight();
    msg("you begin to feel better");
}

void Healing::AffectMonster(Monster* monster)
{
    monster->increase_hp(rnd(8), true, false);
}

void SeeInvisiblePotion::Quaff()
{
    if (!game->hero().sees_invisible()) {
        fuse(unsee_invisible, 0, SEE_DURATION);
        look(false);
        show_invisible();
    }
    sight();
    msg("this potion tastes like %s juice", game->get_environment("fruit").c_str());
}

void GainStrength::Quaff()
{
    discover();
    game->hero().adjust_strength(1);
    msg("you feel stronger. What bulging muscles!");
}

void Poison::Quaff()
{
    const char* sick = "you feel %s sick.";

    discover();
    if (game->hero().adjust_strength(-(rnd(3) + 1))) {
        msg(sick, "very");
    }
    else
        msg(sick, "momentarily");
}

bool Poison::IsEvil() const
{
    return true;
}

ItemCategory Confusion::info;
ItemCategory Paralysis::info;
ItemCategory Poison::info;
ItemCategory GainStrength::info;
ItemCategory SeeInvisiblePotion::info;
ItemCategory Healing::info;
ItemCategory MonsterDetection::info;
ItemCategory MagicDetection::info;
ItemCategory RaiseLevel::info;
ItemCategory ExtraHealing::info;
ItemCategory HasteSelf::info;
ItemCategory RestoreStrength::info;
ItemCategory Blindness::info;
ItemCategory ThirstQuenching::info;
