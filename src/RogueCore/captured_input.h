#pragma once
#include <memory>
#include <vector>
#include "input_interface.h"

struct CapturedInput : public InputInterfaceEx
{
    CapturedInput(std::shared_ptr<InputInterfaceEx> d);

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);

    std::shared_ptr<InputInterfaceEx> m_delegate;
    std::vector<char> m_stream;

    bool m_save_pending = false;
};