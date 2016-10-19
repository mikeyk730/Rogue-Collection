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
    virtual void Quaff() override;
    virtual void AffectMonster(Monster* m) override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct Paralysis : public PotionEx<Paralysis>
{
    virtual void Quaff() override;
    virtual void AffectMonster(Monster* m) override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct Poison : public PotionEx<Poison>
{
    virtual void Quaff() override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct GainStrength : public PotionEx<GainStrength>
{
    virtual void Quaff() override;
    static ItemCategory info;
};

struct SeeInvisiblePotion : public PotionEx<SeeInvisiblePotion>
{
    virtual void Quaff() override;
    static ItemCategory info;
};

struct Healing : public PotionEx<Healing>
{
    virtual void Quaff() override;
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct MonsterDetection : public PotionEx<MonsterDetection>
{
    virtual void Quaff() override;
    static ItemCategory info;
};

struct MagicDetection : public PotionEx<MagicDetection>
{
    virtual void Quaff() override;
    static ItemCategory info;
};

struct RaiseLevel : public PotionEx<RaiseLevel>
{
    virtual void Quaff() override;
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct ExtraHealing : public PotionEx<ExtraHealing>
{
    virtual void Quaff() override;
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct HasteSelf : public PotionEx<HasteSelf>
{
    virtual void Quaff() override;
    virtual void AffectMonster(Monster* m) override;
    static ItemCategory info;
};

struct RestoreStrength : public PotionEx<RestoreStrength>
{
    virtual void Quaff() override;
    static ItemCategory info;
};

struct Blindness : public PotionEx<Blindness>
{
    virtual void Quaff() override;
    virtual void AffectMonster(Monster* m) override;
    virtual bool IsEvil() const override;
    static ItemCategory info;
};

struct ThirstQuenching : public PotionEx<ThirstQuenching>
{
    virtual void Quaff() override;
    static ItemCategory info;
};
