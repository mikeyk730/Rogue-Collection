#pragma once
#include <Windows.h>
#include "RogueCore\display_interface.h"
#include "RogueCore\coord.h"

struct WindowsConsole : public DisplayInterface
{
    WindowsConsole(Coord origin);
    ~WindowsConsole();
    virtual void Draw(_CHAR_INFO* info, Coord dimensions);
    virtual void Draw(_CHAR_INFO* info, Coord dimensions, _SMALL_RECT rect);
    virtual void MoveCursor(Coord pos);
    virtual void SetCursor(bool enable);

private:
    Coord m_origin;
    HANDLE hConsole;
};
