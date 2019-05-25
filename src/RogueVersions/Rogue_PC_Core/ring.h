#pragma once
#include "item.h"

struct Monster;

struct Ring : public Item
{
    Ring();
    Ring(int level);

    virtual std::string TypeName() const override;
    virtual std::string InventoryName() const override;
    virtual bool IsMagic() const override;
    virtual bool IsEvil() const override;
    virtual int Worth() const override;

    virtual int GetFoodCost() = 0;
    virtual void PutOn();
    virtual void Remove();
    virtual bool UsesBonuses() const;
    virtual void OnNewMonster(Monster* monster);
    virtual void OnTurn();

    virtual bool SustainsStrength();
    virtual bool SustainsArmor();
    virtual bool AddsStealth();

    virtual int GetArmorBoost();
    virtual int GetStrBoost();
    virtual int GetHitBoost();
    virtual int GetDmgBoost();
    virtual int GetSaveBoost();
    virtual int GetHpBoost();

    int get_ring_level() const;

protected:
    short ring_level = 0;
};

//ring_eat: How much food does this ring use up?
int ring_eat(int hand);

//ring_num: Print ring bonuses
const char *ring_num(const Ring *obj);
