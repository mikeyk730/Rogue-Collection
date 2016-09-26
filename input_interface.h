#pragma once
#include <string>

struct InputInterface
{
    virtual bool HasMoreInput() = 0;
    virtual char GetNextChar() = 0;
    virtual std::string GetNextString(int size) = 0;

    virtual void Serialize(std::ostream& out) = 0;

    virtual ~InputInterface() {}
};
