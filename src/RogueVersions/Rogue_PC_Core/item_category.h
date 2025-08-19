#pragma once

#include <set>
#include <string>
#include <vector>
#include <map>

struct MagicItem;
struct Item;

struct ItemCategory
{
    std::string name() const;
    void name(const std::string& n);

    std::string guess() const;
    void guess(const std::string& g);

    std::string identifier() const;
    void identifier(const std::string& id);

    std::string kind() const;
    void kind(const std::string& k);

    int worth() const;
    void worth(int w);

    bool is_discovered() const;
    void discover();

private:
    std::string m_name;
    std::string m_identifier;
    std::string m_guess;
    std::string m_kind;
    int m_worth = 0;
    bool m_is_discovered = false;
};
