#pragma once
#include "display_interface_types.h"
#include <cstdint>
#include <string>

struct DisplayInterface
{
    virtual ~DisplayInterface();

    virtual void SetDimensions(Coord dimensions) = 0;
    virtual void UpdateRegion(uint32_t* buf, char* dirty) = 0;
    virtual void MoveCursor(Coord pos) = 0;
    virtual void SetCursor(bool enable) = 0;

    virtual void PlaySound(const std::string& id) = 0;
};
