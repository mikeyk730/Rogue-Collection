#include "gold.h"

Gold::Gold(int value) :
    Item(GOLD, 0)
{
    gold_value = value;
}

Item * Gold::Clone() const
{
    return new Gold(*this);
}

std::string Gold::Name() const
{
    return "gold";
}

std::string Gold::InventoryName() const
{
    sprintf(prbuf, "%d gold", get_gold_value());
    return prbuf;
}

bool Gold::IsMagic() const
{
    return false;
}

bool Gold::IsEvil() const
{
    return false;
}

int Gold::Worth() const
{
    return gold_value;
}

int Gold::get_gold_value() const
{
    return gold_value;
}
