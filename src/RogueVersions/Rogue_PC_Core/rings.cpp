//Routines dealing specifically with rings
//rings.c     1.4 (AI Design) 12/13/84
#include <stdio.h>
#include <sstream>
#include <cstring>
#include "rogue.h"
#include "random.h"
#include "agent.h"
#include "game_state.h"
#include "item_category.h"
#include "daemons.h"
#include "daemon.h"
#include "ring.h"
#include "pack.h"
#include "misc.h"
#include "potion.h"
#include "io.h"
#include "things.h"
#include "weapons.h"
#include "hero.h"
#include "rings.h"
#include "level.h"
#include "monster.h"

char ring_buf[6];

//ring_eat: How much food does this ring use up?
int ring_eat(int hand)
{
    if (game->hero().get_ring(hand) == NULL)
        return 0;
    return game->hero().get_ring(hand)->GetFoodCost();
}

//ring_num: Print ring bonuses
char *ring_num(const Ring *obj)
{
    if (!obj->is_known() && !game->wizard().reveal_items())
        return "";

    if (obj->UsesBonuses())
    {
        ring_buf[0] = ' ';
        strcpy(&ring_buf[1], num(obj->get_ring_level(), 0, (char)RING));
        return ring_buf;
    }
    return "";
}


Ring::Ring() :
    Item(RING, 0)
{
}

/*
Ring::Ring(int level) :
    Item(RING, 0)
{
    ring_level = level;
    if (ring_level < 0)
        set_cursed();
}
*/

std::string Ring::TypeName() const
{
    return "ring";
}

std::string Ring::InventoryName() const
{
    std::ostringstream ss;

    ItemCategory& info = *Category();
    std::string stone = info.identifier();

    if (info.is_discovered() || game->wizard().reveal_items()) {
        ss << "A" << ring_num(this) << " ring of " << info.name();
        if (!short_msgs())
            ss << "(" << stone << ")";
    }
    else if (!info.guess().empty()) {
        ss << "A ring called " << info.guess();
        if (!short_msgs())
            ss << "(" << stone << ")";
    }
    else
        ss << "A" << vowelstr(stone.c_str()) << " " << stone << " ring";

    return ss.str();
}

bool Ring::IsMagic() const
{
    return true;
}

bool Ring::IsEvil() const
{
    if (UsesBonuses())
        return (get_ring_level() < 0);

    return false;
}

int Ring::Worth() const
{
    int worth = Category()->worth();
    if (UsesBonuses()) {
        if (get_ring_level() > 0)
            worth += get_ring_level() * 100;
        else
            worth = 10;
    }
    if (!is_known())
        worth /= 2;
    return worth;
}


int Ring::get_ring_level() const
{
    return ring_level;
}

void Ring::OnTurn()
{
}

bool Ring::SustainsStrength()
{
    return false;
}

bool Ring::SustainsArmor()
{
    return false;
}

bool Ring::AddsStealth()
{
    return false;
}

int Ring::GetArmorBoost()
{
    return 0;
}

int Ring::GetStrBoost()
{
    return 0;
}

int Ring::GetHitBoost()
{
    return 0;
}

int Ring::GetDmgBoost()
{
    return 0;
}

int Ring::GetSaveBoost()
{
    return 0;
}

int Searching::GetFoodCost()
{
    return (rnd(5) == 0);
}

void Searching::OnTurn()
{
    do_search(); //mdk: we search even if asleep!
}

TeleportationRing::TeleportationRing()
{
    set_cursed();
}

int TeleportationRing::GetFoodCost()
{
    return 0;
}

void TeleportationRing::OnTurn()
{
    if (rnd(50) == 17)
        game->hero().teleport();
}

bool TeleportationRing::IsEvil() const
{
    return true;
}

bool Ring::UsesBonuses() const
{
    return false;
}

void Ring::OnNewMonster(Monster * monster)
{
}

Protection::Protection()
{
    if ((ring_level = rnd(3)) == 0) {
        ring_level = -1;
        set_cursed();
    }
}

int Protection::GetFoodCost()
{
    return 1;
}

bool Protection::UsesBonuses() const
{
    return true;
}

int Protection::GetArmorBoost()
{
    return get_ring_level();
}

int Protection::GetSaveBoost()
{
    return get_ring_level();
}

AddStrength::AddStrength()
{
    if ((ring_level = rnd(3)) == 0) {
        ring_level = -1;
        set_cursed();
    }
}

int AddStrength::GetFoodCost()
{
    return 1;
}

bool AddStrength::UsesBonuses() const
{
    return true;
}

int AddStrength::GetStrBoost()
{
    return get_ring_level();
}

Dexterity::Dexterity()
{
    if ((ring_level = rnd(3)) == 0) {
        ring_level = -1;
        set_cursed();
    }
}

int Dexterity::GetFoodCost()
{
    return (rnd(3) == 0);
}

bool Dexterity::UsesBonuses() const
{
    return true;
}

int Dexterity::GetHitBoost()
{
    return get_ring_level();
}

IncreaseDamage::IncreaseDamage()
{
    if ((ring_level = rnd(3)) == 0) {
        ring_level = -1;
        set_cursed();
    }
}

int IncreaseDamage::GetFoodCost()
{
    return (rnd(3) == 0);
}

bool IncreaseDamage::UsesBonuses() const
{
    return true;
}

int IncreaseDamage::GetDmgBoost()
{
    return get_ring_level();
}

void Ring::PutOn()
{
}

void Ring::Remove()
{
}

int SeeInvisibleRing::GetFoodCost()
{
    return (rnd(5) == 0);
}

void SeeInvisibleRing::PutOn()
{
    show_invisible();
}

void SeeInvisibleRing::Remove()
{
    unsee_invisible();
    extinguish(unsee_invisible);
}

int Ring::GetHpBoost()
{
    return 0;
}

int Regeneration::GetHpBoost()
{
    return 1;
}

bool AggravateMonster::IsEvil() const
{
    return true;
}

AggravateMonster::AggravateMonster()
{
    set_cursed();
}

int AggravateMonster::GetFoodCost()
{
    return 0;
}

void AggravateMonster::PutOn()
{
    game->level().aggravate_monsters();
}

void AggravateMonster::OnNewMonster(Monster* monster)
{
    monster->start_run();
}

int Regeneration::GetFoodCost()
{
    return 2;
}

int SlowDigestion::GetFoodCost()
{
    return -rnd(2);
}

int SustainStrength::GetFoodCost()
{
    return 1;
}

bool SustainStrength::SustainsStrength()
{
    return true;
}

int MaintainArmor::GetFoodCost()
{
    return 1;
}

bool MaintainArmor::SustainsArmor()
{
    return true;
}

int Stealth::GetFoodCost()
{
    return 1;
}

bool Stealth::AddsStealth()
{
    return true;
}

int Adornment::GetFoodCost()
{
    return 0;
}

ItemCategory Protection::info;
ItemCategory AddStrength::info;
ItemCategory SustainStrength::info;
ItemCategory Searching::info;
ItemCategory SeeInvisibleRing::info;
ItemCategory Adornment::info;
ItemCategory AggravateMonster::info;
ItemCategory Dexterity::info;
ItemCategory IncreaseDamage::info;
ItemCategory Regeneration::info;
ItemCategory SlowDigestion::info;
ItemCategory TeleportationRing::info;
ItemCategory Stealth::info;
ItemCategory MaintainArmor::info;
