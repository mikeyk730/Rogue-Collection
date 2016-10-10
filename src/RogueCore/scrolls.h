#pragma once
#include "scroll.h"
#include "item_class.h"

template <typename Derived>
struct ScrollEx : public Scroll {
    virtual Item* Clone() const override {
        return new Derived(static_cast<Derived const&>(*this));
    }

    virtual ItemCategory& Info() const
    {
        return Derived::info;
    }
};

struct MonsterConfusion : public ScrollEx<MonsterConfusion>
{
    virtual void Read();
    static ItemCategory info;
};

struct MagicMapping : public ScrollEx<MagicMapping>
{
    virtual void Read();
    static ItemCategory info;
};

struct HoldMonster : public ScrollEx<HoldMonster>
{
    virtual void Read();
    static ItemCategory info;
};

struct Sleep : public ScrollEx<Sleep>
{
    virtual void Read();
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct EnchantArmor : public ScrollEx<EnchantArmor>
{
    virtual void Read();
    static ItemCategory info;
};

struct Identify : public ScrollEx<Identify>
{
    virtual void Read();
    static ItemCategory info;
};

struct ScareMonster : public ScrollEx<ScareMonster>
{
    virtual void Read();
    static ItemCategory info;
};

struct FoodDetection : public ScrollEx<FoodDetection>
{
    virtual void Read();
    static ItemCategory info;
};

struct Teleportation : public ScrollEx<Teleportation>
{
    virtual void Read();
    static ItemCategory info;
};

struct EnchantWeapon : public ScrollEx<EnchantWeapon>
{
    virtual void Read();
    static ItemCategory info;
};

struct CreateMonster : public ScrollEx<CreateMonster>
{
    virtual void Read();
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct RemoveCurse : public ScrollEx<RemoveCurse>
{
    virtual void Read();
    static ItemCategory info;
};

struct AggravateMonsters : public ScrollEx<AggravateMonsters>
{
    virtual void Read();
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct BlankPaper : public ScrollEx<BlankPaper>
{
    virtual void Read();
    static ItemCategory info;
};

struct VorpalizeWeapon : public ScrollEx<VorpalizeWeapon>
{
    virtual void Read();
    static ItemCategory info;
};

int is_scare_monster_scroll(Item* item);
