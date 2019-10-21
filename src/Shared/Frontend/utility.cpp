#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <ctime>
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

    std::time_t now;
    std::time(&now);

    std::tm* timeinfo = std::localtime(&now);

    char buffer [80];
    std::strftime(buffer,80,"%Y-%m-%d-%H-%M-%S",timeinfo);

    return buffer;
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

bool BlinkChar(uint32_t ch)
{
    return CharText(ch) == STAIRS;
}

uint32_t GetUnixChar(uint32_t c)
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

uint32_t GetTileColor(int chr, int attr)
{
    //if it is inside a room
    if (attr == 0x07 || attr == 0)
    switch (chr)
    {
    case DOOR:
    case VWALL:
    case HWALL:
    case ULWALL:
    case URWALL:
    case LLWALL:
    case LRWALL:
        return 0x06; //brown
    case FLOOR:
        return 0x0a; //light green
    case STAIRS:
        return 0x20; //black on light green
    case TRAP:
        return 0x05; //magenta
    case GOLD:
    case PLAYER:
        return 0x0e; //yellow
    case POTION:
    case SCROLL:
    case STICK:
    case ARMOR:
    case AMULET:
    case RING:
    case WEAPON:
        return 0x09; //light blue
    case FOOD:
        return 0x04; //red
    }
    //if inside a passage or a maze
    else if (attr == 0x70)
    switch (chr)
    {
    case FOOD:
        return 0x74; //red on grey
    case GOLD:
    case PLAYER:
        return 0x7e; //yellow on grey
    case POTION:
    case SCROLL:
    case STICK:
    case ARMOR:
    case AMULET:
    case RING:
    case WEAPON:
        return 0x71; //blue on grey
    }

    return attr;
}

uint32_t FlipColor(uint32_t c)
{
    return ((c & 0x0f) << 4) | ((c & 0xf0) >> 4);
}
