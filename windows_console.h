#pragma once

#include <Windows.h>
#include "screen_interface.h"
#include "coord.h"

struct WindowsConsole : public ScreenInterface
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
