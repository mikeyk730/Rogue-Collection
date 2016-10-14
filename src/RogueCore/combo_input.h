#pragma once
#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "input_interface.h"

struct ComboInput : public InputInterfaceEx
{
    ComboInput(std::shared_ptr<InputInterfaceEx> primary, std::shared_ptr<InputInterfaceEx> backup);

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);

    bool Swap();

private:
    std::shared_ptr<InputInterfaceEx> m_active;
    std::shared_ptr<InputInterfaceEx> m_backup;
};
