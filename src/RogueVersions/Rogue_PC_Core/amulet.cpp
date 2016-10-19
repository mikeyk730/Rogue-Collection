#include "amulet.h"
#include "rogue.h"

Amulet::Amulet() : 
    Item(AMULET, 0)
{
}

Item * Amulet::Clone() const
{
    return new Amulet(*this);
}

std::string Amulet::TypeName() const
{
    return "amulet";
}

std::string Amulet::InventoryName() const
{
    return "The Amulet of Yendor";
}

bool Amulet::IsMagic() const
{
    return true;
}

bool Amulet::IsEvil() const
{
    return false;
}

int Amulet::Worth() const
{
    return 1000;
}
