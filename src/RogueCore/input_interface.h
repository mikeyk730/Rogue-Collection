#pragma once
#include <string>
#include <vector>
#include <functional>

struct InputInterface
{
    virtual ~InputInterface();

    virtual bool HasMoreInput() = 0;
    virtual char GetNextChar() = 0;
    virtual std::string GetNextString(int size) = 0;

    virtual bool IsCapsLockOn() = 0;
    virtual bool IsNumLockOn() = 0;
    virtual bool IsScrollLockOn() = 0;
 
    virtual void Serialize(std::ostream& out) = 0;
};
