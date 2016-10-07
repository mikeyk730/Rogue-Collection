#pragma once
#include "RogueCore/input_interface.h"

struct ConsoleKeyboardInput : public InputInterface
{
    ConsoleKeyboardInput();

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual bool IsCapsLockOn();
    virtual bool IsNumLockOn();
    virtual bool IsScrollLockOn();

    virtual void Serialize(std::ostream& out);
};
