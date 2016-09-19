#include "input_interface.h"

struct KeyboardInput : public InputInterface
{
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);
};
