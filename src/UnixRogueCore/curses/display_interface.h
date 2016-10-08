#pragma once
#include "display_interface_types.h"

struct Coord;

struct DisplayInterface
{
    virtual ~DisplayInterface() = 0;

    virtual void SetDimensions(Coord dimensions) = 0;
    virtual void Draw(CharInfo* buf, bool*) = 0;
    virtual void Draw(CharInfo* buf, bool*, Region rect) = 0;
    virtual void MoveCursor(Coord pos) = 0;
    virtual void SetCursor(bool enable) = 0;
};
