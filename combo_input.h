#pragma once
#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <windows.h>
#include "input_interface.h"

struct ComboInput : public InputInterface
{
    ComboInput(InputInterface* primary, InputInterface* backup);

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);

    virtual void OnReplayEnd(const std::function<void()>& handler);
    virtual void OnFastPlayChanged(const std::function<void(bool)>& handler);

    bool Swap();

private:
    std::unique_ptr<InputInterface> m_active;
    std::unique_ptr<InputInterface> m_backup;
};
