#pragma once
#include <memory>
#include "RogueCore/display_interface.h"
#include "RogueCore/input_interface.h"

struct Region;
struct Coord;

struct SdlRogue : public DisplayInterface, public InputInterface
{
    SdlRogue();
    ~SdlRogue();

    void Run();
    void Quit();

    //display interface
    virtual void SetDimensions(Coord dimensions);
    virtual void Draw(CharInfo* info, bool* text_mask);
    virtual void Draw(CharInfo* info, bool* text_mask, Region rect);
    virtual void MoveCursor(Coord pos);
    virtual void SetCursor(bool enable);

    //input interface
    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);
    virtual bool IsCapsLockOn();
    virtual bool IsNumLockOn();
    virtual bool IsScrollLockOn();
    virtual void Serialize(std::ostream& out);

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
