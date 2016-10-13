#pragma once
#include <cstdint>

struct Coord
{
    int x;
    int y;
};

struct Region
{
    int Left;
    int Top;
    int Right;
    int Bottom;

    inline int Width() { return Right - Left + 1; }
    inline int Height() { return Bottom - Top + 1; }
};

typedef uint32_t CursesDataType;