#pragma once
#include "display_interface.h"

struct WindowsConsole : public DisplayInterface
{
    WindowsConsole(Coord origin);
    ~WindowsConsole();

    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(CursesDataType** buf, Region rect) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;

private:
    Coord m_origin;
    Coord m_dimensions;
    void* hConsole;
};
