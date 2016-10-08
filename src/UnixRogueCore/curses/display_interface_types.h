#pragma once

#ifdef _WIN32
#include <Windows.h>
typedef CHAR_INFO CharInfo;
#elif
struct CharInfo {
    union {
        wchar_t UnicodeChar;
        char   AsciiChar;
    } Char;
    short Attributes;
};
#endif

struct Region
{
    int Left;
    int Top;
    int Right;
    int Bottom;
};
