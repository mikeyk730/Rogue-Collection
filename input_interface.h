#pragma once
#include <string>
#include <vector>
#include <functional>

struct InputInterface
{
    virtual ~InputInterface() {}

    virtual bool HasMoreInput() = 0;
    virtual char GetNextChar() = 0;
    virtual std::string GetNextString(int size) = 0;

    virtual void Serialize(std::ostream& out) = 0;

    virtual void OnReplayEnd(const std::function<void()>& handler) = 0;
    virtual void OnFastPlayChanged(const std::function<void(bool)>& handler) = 0;
};
