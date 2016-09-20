#pragma once
#include <memory>
#include <vector>
#include "input_interface.h"

struct CapturedInput : public InputInterface
{
    CapturedInput(InputInterface* d);

    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);

    std::unique_ptr<InputInterface> m_delegate;
    std::vector<char> m_stream;
};