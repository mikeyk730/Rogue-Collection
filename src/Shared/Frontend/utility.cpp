#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <cstring>
#include <map>
#include <pc_gfx_charmap.h>
#include "utility.h"

std::ostream& WriteShortString(std::ostream& out, const std::string& s)
{
    assert(s.length() < 255);
    unsigned char length = static_cast<unsigned char>(s.length());
    Write<unsigned char>(out, length);

    out.write(s.c_str(), length);
    return out;
}

std::istream& ReadShortString(std::istream& in, std::string* s)
{
    unsigned char length;
    Read<unsigned char>(in, &length);

    char buf[255];
    memset(buf, 0, 255);
    in.read(buf, length);
    *s = buf;

    return in;
}

void throw_error(const std::string &msg)
{
    throw std::runtime_error(msg);
}

void Delay(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string GetTimestamp()
{
    time_t now;
    time(&now);

    tm t;
    localtime_s(&t, &now);

    char str[200];
    strftime(str, 200, "%FT%H-%M-%S", &t);
    return str;
}

namespace
{
    std::map<int, int> unix_chars = {
        { PASSAGE,   '#' },
        { DOOR,      '+' },
        { FLOOR,     '.' },
        { PLAYER,    '@' },
        { TRAP,      '^' },
        { STAIRS,    '%' },
        { GOLD,      '*' },
        { POTION,    '!' },
        { SCROLL,    '?' },
        { FOOD,      ':' },
        { STICK,     '/' },
        { ARMOR,     ']' },
        { AMULET,    ',' },
        { RING,      '=' },
        { WEAPON,    ')' },
        { VWALL,     '|' },
        { HWALL,     '-' },
        { ULWALL,    '-' },
        { URWALL,    '-' },
        { LLWALL,    '-' },
        { LRWALL,    '-' },
        { 204,       '|' },
        { 185,       '|' },
    };
}

uint32_t CharText(uint32_t ch)
{
    return ch & 0x0000ffff;
}

uint32_t CharColor(uint32_t ch)
{
    return (ch >> 24) & 0xff;
}

bool IsText(uint32_t ch)
{
    return (ch & 0x010000) == 0;
}

char GetUnixChar(unsigned char c)
{
    auto i = unix_chars.find(c);
    if (i != unix_chars.end())
        return i->second;

    return c;
}

char GetRawCharFromData(uint32_t* data, int r, int c, int cols)
{
    unsigned char ch = CharText(data[r*cols + c]);
    auto i = unix_chars.find(ch);
    if (i != unix_chars.end())
        ch = i->second;
    return (ch != 0 ? ch : ' ');
}
