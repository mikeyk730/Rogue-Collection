#pragma once

#include <Windows.h>
#include "rogue.h"

struct ScreenInterface
{
    virtual void Draw(CHAR_INFO* buf) = 0;
    virtual void Draw(CHAR_INFO* buf, SMALL_RECT rect) = 0;
    virtual void MoveCursor(Coord pos) = 0;
    virtual void SetCursor(bool enable) = 0;
};
