#pragma once

#include <set>
#include <vector>
#include <map>

struct MagicItem;
struct Item;

//todo: reimplement so just has one vector<record>
struct ItemClass
{
    virtual ~ItemClass();

    std::string get_name(int type) const;
    std::string get_identifier(int type) const;
    virtual std::string get_inventory_name(Item* obj) const = 0;
    
    int get_probability(int type) const;
    int get_value(int type) const;

    bool is_discovered(int type) const;
    void discover(int type);

    std::string get_guess(int type) const;
    void set_guess(int type, const std::string& guess);
    void call_it(int type); //Call an object something after use.

    int get_max_items() const;

public:
    std::vector<MagicItem> m_magic_props;
protected:
    std::vector<std::string> m_identifier;
private:
    std::set<int> m_discoveries;
    std::map<int, std::string> m_guesses;
};

struct ScrollInfo : public ItemClass
{
    ScrollInfo();
    virtual std::string get_inventory_name(Item* obj) const;
};

struct PotionInfo : public ItemClass
{
    PotionInfo();
    virtual std::string get_inventory_name(Item* obj) const;
};

struct StickInfo : public ItemClass
{
    StickInfo();
    virtual std::string get_inventory_name(Item* obj) const;

    bool is_staff(int which) const;
    std::string get_type(int which) const;

    std::vector<std::string> m_type;
};

struct RingInfo : public ItemClass
{
    RingInfo();
    virtual std::string get_inventory_name(Item* obj) const;
};
