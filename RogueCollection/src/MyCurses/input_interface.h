#pragma once
#include <string>

struct InputInterface
{
    virtual ~InputInterface();

    virtual char GetChar(bool block) = 0;
};
