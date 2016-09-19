#pragma once
#include "input_interface.h"

struct KeyboardInput : public InputInterface
{
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);
};
