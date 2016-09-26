#pragma once
#include "input_interface.h"

struct KeyboardInput : public InputInterface
{
    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);
};
