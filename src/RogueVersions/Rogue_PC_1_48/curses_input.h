#pragma once
#include <input_interface_ex.h>
#include <input_interface.h>

struct ConsoleKeyboardInput : public InputInterfaceEx
{
    ConsoleKeyboardInput(InputInterface* i);

    virtual bool HasMoreInput() override;
    virtual char GetNextChar(bool *is_replay) override;
    virtual std::string GetNextString(int size) override;

    virtual void Serialize(std::ostream& out);

    InputInterface* m_impl;
};
