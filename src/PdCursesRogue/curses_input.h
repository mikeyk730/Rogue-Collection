#pragma once
#include "RogueCore/input_interface.h"

struct ConsoleKeyboardInput : public InputInterfaceEx
{
    ConsoleKeyboardInput();

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);
};
