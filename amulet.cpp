#include "amulet.h"

Amulet::Amulet() : Item(AMULET, 0)
{
    this->hit_plus = 0;
    this->damage_plus = 0;
    this->damage = "0d0";
    this->throw_damage = "0d0";
    this->armor_class = 11;
}

Item * Amulet::Clone() const
{
    return new Amulet(*this);
}

std::string Amulet::Name() const
{
    return "amulet";
}

std::string Amulet::InventoryName() const
{
    return "The Amulet of Yendor";
}

bool Amulet::IsEvil() const
{
    return false;
}

int Amulet::Worth() const
{
    return 1000;
}
