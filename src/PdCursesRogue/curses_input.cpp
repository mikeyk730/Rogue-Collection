#include <curses.h>
#include "curses_input.h"

ConsoleKeyboardInput::ConsoleKeyboardInput()
{
}

bool ConsoleKeyboardInput::HasMoreInput()
{
    return true;
}

char ConsoleKeyboardInput::GetNextChar() {
    return ::getch();
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
