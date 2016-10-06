#pragma once

#include <Windows.h>
#include "screen_interface.h"
#include "coord.h"

struct WindowsConsole : public ScreenInterface
{
    WindowsConsole(Coord origin, Coord dimensions);
    ~WindowsConsole();
    virtual void Draw(_CHAR_INFO* info);
    virtual void Draw(_CHAR_INFO* info, _SMALL_RECT rect);
    virtual void MoveCursor(Coord pos);
    virtual void SetCursor(bool enable);

private:
    Coord m_origin;
    Coord m_dimensions;
    HANDLE hConsole;
};
