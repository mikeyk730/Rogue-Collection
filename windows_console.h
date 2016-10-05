#pragma once

#include "screen_interface.h"

struct WindowsConsole : public ScreenInterface
{
    WindowsConsole(Coord origin, Coord dimensions);
    ~WindowsConsole();
    virtual void Draw(CHAR_INFO* info);
    virtual void Draw(CHAR_INFO* info, SMALL_RECT rect);
    virtual void MoveCursor(Coord pos);
    virtual void SetCursor(bool enable);

private:
    Coord m_origin;
    Coord m_dimensions;
    HANDLE hConsole;
};
