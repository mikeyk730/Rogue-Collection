#pragma once
#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "input_interface_ex.h"

struct ComboInput : public InputInterfaceEx
{
    ComboInput(std::shared_ptr<InputInterfaceEx> primary, std::shared_ptr<InputInterfaceEx> backup);

    virtual bool HasMoreInput() override;
    virtual char GetNextChar(bool *is_replay) override;
    virtual std::string GetNextString(int size) override;

    virtual void Serialize(std::ostream& out);

    bool Swap();

private:
    std::shared_ptr<InputInterfaceEx> m_active;
    std::shared_ptr<InputInterfaceEx> m_backup;
};
