#pragma once
#include "potion.h"

template <typename Derived>
struct PotionEx : public Potion {
    virtual Item* Clone() const override {
        return new Derived(static_cast<Derived const&>(*this));
    }

    virtual ItemCategory* Category() const override
    {
        return &Derived::info;
    }
};

struct Confusion : public PotionEx<Confusion>
{
    virtual void Quaff();
    virtual void AffectMonster(Monster* m) override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct Paralysis : public PotionEx<Paralysis>
{
    virtual void Quaff();
    virtual void AffectMonster(Monster* m) override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct Poison : public PotionEx<Poison>
{
    virtual void Quaff();
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct GainStrength : public PotionEx<GainStrength>
{
    virtual void Quaff();
    static ItemCategory info;
};

struct SeeInvisible : public PotionEx<SeeInvisible>
{
    virtual void Quaff();
    static ItemCategory info;
};

struct Healing : public PotionEx<Healing>
{
    virtual void Quaff();
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct MonsterDetection : public PotionEx<MonsterDetection>
{
    virtual void Quaff();
    static ItemCategory info;
};

struct MagicDetection : public PotionEx<MagicDetection>
{
    virtual void Quaff();
    static ItemCategory info;
};

struct RaiseLevel : public PotionEx<RaiseLevel>
{
    virtual void Quaff();
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct ExtraHealing : public PotionEx<ExtraHealing>
{
    virtual void Quaff();
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct HasteSelf : public PotionEx<HasteSelf>
{
    virtual void Quaff();
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct RestoreStrength : public PotionEx<RestoreStrength>
{
    virtual void Quaff();
    static ItemCategory info;
};

struct Blindness : public PotionEx<Blindness>
{
    virtual void Quaff();
    virtual void AffectMonster(Monster* m) override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct ThirstQuenching : public PotionEx<ThirstQuenching>
{
    virtual void Quaff();
    static ItemCategory info;
};
