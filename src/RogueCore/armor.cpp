//This file contains misc functions for dealing with armor
//@(#)armor.c 1.2 (AI Design) 2/12/84

#include <stdio.h>

#include "random.h"
#include "game_state.h"
#include "armor.h"
#include "io.h"
#include "pack.h"
#include "things.h"
#include "daemon.h"
#include "weapons.h"
#include "main.h"
#include "hero.h"

//Names of armor types
const char *a_names[MAXARMORS] =
{
  "leather armor",
  "ring mail",
  "studded leather armor",
  "scale mail",
  "chain mail",
  "splint mail",
  "banded mail",
  "plate mail"
};

//Chance for each armor type
int a_chances[MAXARMORS] =
{
  20,
  35,
  50,
  63,
  75,
  85,
  95,
  100
};

//Armor class for each armor type
int a_class[MAXARMORS] =
{
  8,
  7,
  7,
  6,
  5,
  4,
  4,
  3
};

int get_default_class(int type)
{
    return a_class[type];
}

Item* create_armor()
{
    int j, k;

    for (j = 0, k = rnd(100); j < MAXARMORS; j++) {
        if (k < a_chances[j])
            break;
    }
    if (j == MAXARMORS) {
        debug("Picked a bad armor %d", k);
        j = 0;
    }
    int which = j;
    return new Armor(which);
}

//todo:does this cause any problems?
//waste_time: Do nothing but let other things happen
void waste_time()
{
    if (!game->hero().decrement_num_actions()) {
        do_daemons();
        do_fuses();
    }
}

std::string Armor::InventoryName() const
{
    char *pb = prbuf;

    if (is_known() || game->wizard().reveal_items())
        chopmsg(pb, "%s %s", "%s %s [armor class %d]", num(get_default_class(m_which) - get_armor_class(), 0, (char)ARMOR),
            Name().c_str(), armor_class_for_display());
    else
        sprintf(pb, "%s", Name().c_str());

    return prbuf;
}

bool Armor::IsMagic() const
{
    return get_armor_class() != get_default_class(m_which);
}

Armor::Armor(int which) :
    Item(ARMOR, which)
{
    armor_class = get_default_class(which);

    int k;
    if ((k = rnd(100)) < 20) {
        set_cursed();
        armor_class += rnd(3) + 1;
    }
    else if (k < 28)
        armor_class -= rnd(3) + 1;
}

Armor::Armor(int which, int ac_mod) :
    Item(ARMOR, which)
{
    if (ac_mod > 0)
        set_cursed();
    armor_class = get_default_class(which) + ac_mod;
}

Item * Armor::Clone() const
{
    return new Armor(*this);
}

std::string Armor::Name() const
{
    return a_names[m_which];
}

bool Armor::IsEvil() const
{
    return get_armor_class() > get_default_class(m_which);
}

int Armor::Worth() const
{
    int worth = 0;
    switch (m_which)
    {
    case LEATHER: worth = 20; break;
    case RING_MAIL: worth = 25; break;
    case STUDDED_LEATHER: worth = 20; break;
    case SCALE_MAIL: worth = 30; break;
    case CHAIN_MAIL: worth = 75; break;
    case SPLINT_MAIL: worth = 80; break;
    case BANDED_MAIL: worth = 90; break;
    case PLATE_MAIL: worth = 150; break;
    }
    worth += (9 - get_armor_class()) * 100;
    worth += (10 * (get_default_class(m_which) - get_armor_class()));
    return worth;
}

int Armor::get_armor_class() const
{
    return armor_class;
}

int Armor::armor_class_for_display() const
{
    int a = get_armor_class();
    return (-((a)-11));
}

void Armor::enchant_armor()
{
    armor_class--;
    remove_curse();
}

void Armor::weaken_armor()
{
    armor_class++;
}

