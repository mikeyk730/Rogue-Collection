#pragma once
#include "stick.h"

template <typename Derived>
struct StickEx : public Stick {
    StickEx() : Stick(Derived::info.kind())
    {}

    virtual Item* Clone() const override {
        return new Derived(static_cast<Derived const&>(*this));
    }

    virtual ItemCategory* Category() const override
    {
        return &Derived::info;
    }
};

struct Light : public StickEx<Light>
{
    Light();
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct Striking : public StickEx<Striking>
{
    Striking();
    virtual bool Zap(Coord dir) override;
    void drain_striking();
    void set_striking_damage();
    static ItemCategory info;
};

struct Lightning : public StickEx<Lightning>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct Fire : public StickEx<Fire>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct Cold : public StickEx<Cold>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct Polymorph : public StickEx<Polymorph>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct MagicMissileStick : public StickEx<MagicMissileStick>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct HasteMonster : public StickEx<HasteMonster>
{
    virtual bool IsEvil() const override;
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct SlowMonster : public StickEx<SlowMonster>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct DrainLife : public StickEx<DrainLife>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct Nothing : public StickEx<Nothing>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct TeleportAway : public StickEx<TeleportAway>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct TeleportTo : public StickEx<TeleportTo>
{
    virtual bool IsEvil() const override;
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};

struct Cancellation : public StickEx<Cancellation>
{
    virtual bool Zap(Coord dir) override;
    static ItemCategory info;
};
