#pragma once
#include <string>

struct InputInterface
{
    virtual ~InputInterface();

    virtual char GetChar() = 0;
    virtual std::string GetString(int size) = 0;
};
