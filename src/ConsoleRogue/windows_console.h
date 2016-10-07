#pragma once
#include <Windows.h>
#include "RogueCore/display_interface.h"
#include "RogueCore/coord.h"

struct WindowsConsole : public DisplayInterface
{
    WindowsConsole(Coord origin);
    ~WindowsConsole();

    virtual void SetDimensions(Coord dimensions);
    virtual void Draw(CharInfo* info, bool*);
    virtual void Draw(CharInfo* info, bool*, Region rect);
    virtual void MoveCursor(Coord pos);
    virtual void SetCursor(bool enable);

private:
    Coord m_origin;
    Coord m_dimensions;
    HANDLE hConsole;
};
