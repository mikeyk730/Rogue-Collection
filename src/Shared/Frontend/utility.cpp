#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <cstring>
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

