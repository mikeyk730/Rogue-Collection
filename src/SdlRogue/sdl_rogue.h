#pragma once
#include <memory>
#include "RogueCore/display_interface.h"
#include "RogueCore/input_interface.h"

struct _CHAR_INFO;
struct _SMALL_RECT;
struct Coord;

struct SdlRogue : public DisplayInterface, public InputInterface
{
    SdlRogue();
    ~SdlRogue();

    void Run();
    void Quit();

    //display interface
    virtual void SetDimensions(Coord dimensions);
    virtual void Draw(_CHAR_INFO* info);
    virtual void Draw(_CHAR_INFO* info, _SMALL_RECT rect);
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
