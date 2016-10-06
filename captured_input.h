#pragma once
#include <memory>
#include <vector>
#include "input_interface.h"

struct CapturedInput : public InputInterface
{
    CapturedInput(std::unique_ptr<InputInterface> d);

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual bool IsCapsLockOn();
    virtual bool IsNumLockOn();
    virtual bool IsScrollLockOn();


    virtual void Serialize(std::ostream& out);

    std::unique_ptr<InputInterface> m_delegate;
    std::vector<char> m_stream;

    bool m_save_pending = false;
};