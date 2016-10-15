#pragma once

struct ConsoleKeyboardInput : public InputInterface
{
    ConsoleKeyboardInput();

    //input interface
    virtual char GetChar(bool block) override;
};
