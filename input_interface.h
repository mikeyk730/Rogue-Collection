#pragma once
#include <string>

struct InputInterface
{
    virtual char GetNextChar() = 0;
    virtual std::string GetNextString(int size) = 0;

    virtual ~InputInterface() {}
};
