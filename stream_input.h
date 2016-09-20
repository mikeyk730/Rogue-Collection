#pragma once
#include <memory>
#include <iostream>
#include "input_interface.h"

struct StreamInput : public InputInterface
{
    StreamInput(std::istream& in, InputInterface* backup);

    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);

    std::unique_ptr<InputInterface> m_backup;
    std::istream& m_stream;
};
