#include "item_category.h"

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

std::string ItemCategory::kind() const
{
    return m_kind;
}

void ItemCategory::kind(const std::string & k)
{
    m_kind = k;
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
