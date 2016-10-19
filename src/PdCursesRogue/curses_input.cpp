#include <curses.h>
#include "curses_input.h"

ConsoleKeyboardInput::ConsoleKeyboardInput(InputInterface * i)
    : m_impl(i)
{
}

bool ConsoleKeyboardInput::HasMoreInput()
{
    return true;
}

char ConsoleKeyboardInput::GetNextChar(bool *is_replay) {
    return m_impl->GetChar(true, false, is_replay);
}

std::string ConsoleKeyboardInput::GetNextString(int size)
{
    char* buf = new char[size];
    ::getnstr(buf, size);
    std::string s(buf);
    delete buf;
    return s;
}

void ConsoleKeyboardInput::Serialize(std::ostream& out)
{ }
