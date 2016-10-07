#pragma once

struct CharInfo {
    union {
        wchar_t UnicodeChar;
        char   AsciiChar;
    } Char;
    short Attributes;
};

struct Region
{
    int Left;
    int Top;
    int Right;
    int Bottom;
};
