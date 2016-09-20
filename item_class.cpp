#include "item_class.h"
#include "rogue.h"
#include "io.h"

ItemClass::~ItemClass()
{ }

std::string ItemClass::get_name(int type) const 
{
    return m_magic_props[type].name;
}

int ItemClass::get_probability(int type) const
{
    return m_magic_props[type].prob;
}

int ItemClass::get_value(int type) const
{
    return m_magic_props[type].worth;
}

std::string ItemClass::get_identifier(int type) const
{
    return m_identifier[type];
}

bool ItemClass::is_discovered(int type) const
{
    return m_discoveries.find(type) != m_discoveries.end();
}

void ItemClass::discover(int type)
{
    m_discoveries.insert(type);
}

std::string ItemClass::get_guess(int type) const
{
    auto i = m_guesses.find(type);
    if (i != m_guesses.end())
        return i->second;
    return "";
}

void ItemClass::set_guess(int type, const std::string& guess)
{
    m_guesses[type] = guess;
}

void ItemClass::call_it2(int type)
{
    if (is_discovered(type))
        set_guess(type, "");
    else if (get_guess(type).empty())
    {
        msg("%scall it? ", noterse("what do you want to "));
        getinfo(prbuf, MAXNAME);
        if (*prbuf != ESCAPE)
            set_guess(type, prbuf);
        msg("");
    }
}

int ItemClass::get_max_items() const
{
    return m_identifier.size();
}
