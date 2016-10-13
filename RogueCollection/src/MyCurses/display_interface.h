#pragma once
#include "display_interface_types.h"
#include <cstdint>

struct Coord;

struct DisplayInterface
{
    virtual ~DisplayInterface();

    virtual void SetDimensions(Coord dimensions) = 0;
    virtual void UpdateRegion(uint32_t** buf, Region rect) = 0;
    virtual void MoveCursor(Coord pos) = 0;
    virtual void SetCursor(bool enable) = 0;
};
