#pragma once

#include"util.h"

struct Region
{
    int Left;
    int Top;
    int Right;
    int Bottom;

    inline int Width() { return Right - Left + 1; }
    inline int Height() { return Bottom - Top + 1; }
};
