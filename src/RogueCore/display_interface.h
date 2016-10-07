#pragma once

struct _CHAR_INFO;
struct Coord;

struct Region
{
    int Left;
    int Top;
    int Right;
    int Bottom;
};

struct DisplayInterface
{
    virtual ~DisplayInterface();

    virtual void SetDimensions(Coord dimensions) = 0;
    virtual void Draw(_CHAR_INFO* buf) = 0;
    virtual void Draw(_CHAR_INFO* buf, Region rect) = 0;
    virtual void MoveCursor(Coord pos) = 0;
    virtual void SetCursor(bool enable) = 0;
};
