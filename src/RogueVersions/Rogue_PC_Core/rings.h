#pragma once
#include "ring.h"

template <typename Derived>
struct RingEx : public Ring {
    virtual Item* Clone() const override {
        return new Derived(static_cast<Derived const&>(*this));
    }

    virtual ItemCategory* Category() const override
    {
        return &Derived::info;
    }
};

struct Protection : RingEx<Protection>
{
    Protection();
    virtual int GetFoodCost() override;
    virtual bool UsesBonuses() const override;
    virtual int GetArmorBoost() override;
    virtual int GetSaveBoost() override;
    static ItemCategory info;
};

struct AddStrength : RingEx<AddStrength>
{
    AddStrength();
    virtual int GetFoodCost() override;
    virtual bool UsesBonuses() const override;
    virtual int GetStrBoost() override;
    static ItemCategory info;
};

struct SustainStrength : RingEx<SustainStrength>
{
    virtual int GetFoodCost() override;
    virtual bool SustainsStrength() override;
    static ItemCategory info;
};

struct Searching : RingEx<Searching>
{
    virtual int GetFoodCost() override;
    virtual void OnTurn() override;
    static ItemCategory info;
};

struct SeeInvisibleRing : RingEx<SeeInvisibleRing>
{
    virtual int GetFoodCost() override;
    virtual void PutOn() override;
    virtual void Remove() override;
    static ItemCategory info;
};

struct Adornment : RingEx<Adornment>
{
    virtual int GetFoodCost() override;
    static ItemCategory info;
};

struct AggravateMonster : RingEx<AggravateMonster>
{
    AggravateMonster();
    virtual int GetFoodCost() override;
    virtual void PutOn() override;
    virtual void OnNewMonster(Monster* monster) override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct Dexterity : RingEx<Dexterity>
{
    Dexterity();
    virtual int GetFoodCost() override;
    virtual bool UsesBonuses() const override;
    virtual int GetHitBoost() override;
    static ItemCategory info;
};

struct IncreaseDamage : RingEx<IncreaseDamage>
{
    IncreaseDamage();
    virtual int GetFoodCost() override;
    virtual bool UsesBonuses() const override;
    virtual int GetDmgBoost() override;
    static ItemCategory info;
};

struct Regeneration : RingEx<Regeneration>
{
    virtual int GetFoodCost() override;
    virtual int GetHpBoost() override;
    static ItemCategory info;
};

struct SlowDigestion : RingEx<SlowDigestion>
{
    virtual int GetFoodCost() override;
    static ItemCategory info;
};

struct TeleportationRing : RingEx<TeleportationRing>
{
    TeleportationRing();
    virtual int GetFoodCost() override;
    virtual void OnTurn() override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct Stealth : RingEx<Stealth>
{
    virtual int GetFoodCost() override;
    virtual bool AddsStealth() override;
    static ItemCategory info;
};

struct MaintainArmor : RingEx<MaintainArmor>
{
    virtual int GetFoodCost() override;
    virtual bool SustainsArmor() override;
    static ItemCategory info;
};
