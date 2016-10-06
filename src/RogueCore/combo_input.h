#pragma once
#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <windows.h>
#include "input_interface.h"

struct ComboInput : public InputInterface
{
    ComboInput(std::shared_ptr<InputInterface> primary, std::shared_ptr<InputInterface> backup);

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual bool IsCapsLockOn();
    virtual bool IsNumLockOn();
    virtual bool IsScrollLockOn();


    virtual void Serialize(std::ostream& out);

    bool Swap();

private:
    std::shared_ptr<InputInterface> m_active;
    std::shared_ptr<InputInterface> m_backup;
};
