#include "item_class.h"
#include "rogue.h"
#include "things.h"
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

void ItemClass::call_it(int type)
{
    char buf[MAXNAME];
    if (is_discovered(type))
        set_guess(type, "");
    else if (get_guess(type).empty())
    {
        msg("%scall it? ", noterse("what do you want to "));
        getinfo(buf, MAXNAME);
        if (*buf != ESCAPE)
            set_guess(type, buf);
        msg("");
    }
}

int ItemClass::get_max_items() const
{
    return m_identifier.size();
}

std::string ItemCategory::name() const
{
    return m_name;
}

void ItemCategory::name(const std::string& n)
{
    m_name = n;
}

std::string ItemCategory::guess() const
{
    return m_guess;
}

void ItemCategory::guess(const std::string & g)
{
    m_guess = g;
}

std::string ItemCategory::identifier() const
{
    return m_identifier;
}

void ItemCategory::identifier(const std::string & id)
{
    m_identifier = id;
}

int ItemCategory::worth() const
{
    return m_worth;
}

void ItemCategory::worth(int w)
{
    m_worth = w;
}

bool ItemCategory::is_discovered() const
{
    return m_is_discovered;
}

void ItemCategory::discover()
{
    m_is_discovered = true;
}
