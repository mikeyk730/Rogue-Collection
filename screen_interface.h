#pragma once

struct _SMALL_RECT;
struct _CHAR_INFO;
struct Coord;

struct ScreenInterface
{
    virtual void Draw(_CHAR_INFO* buf) = 0;
    virtual void Draw(_CHAR_INFO* buf, _SMALL_RECT rect) = 0;
    virtual void MoveCursor(Coord pos) = 0;
    virtual void SetCursor(bool enable) = 0;
};
