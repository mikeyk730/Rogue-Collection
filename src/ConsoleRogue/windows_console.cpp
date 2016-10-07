#include "windows_console.h"

#pragma warning(disable:4838)

WindowsConsole::WindowsConsole(Coord origin) :
    m_origin(origin)
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

WindowsConsole::~WindowsConsole()
{
    CloseHandle(hConsole);
}

void WindowsConsole::SetDimensions(Coord dimensions)
{
    m_dimensions = dimensions;
}

void WindowsConsole::Draw(_CHAR_INFO* info)
{
    COORD dwBufferSize = { m_dimensions.x, m_dimensions.y };
    COORD dwBufferCoord = { 0, 0 };
    SMALL_RECT rcRegion = { m_origin.x, m_origin.y, m_origin.x + m_dimensions.x - 1, m_origin.y + m_dimensions.y - 1 };
    WriteConsoleOutput(hConsole, info, dwBufferSize, dwBufferCoord, &rcRegion);
}

void WindowsConsole::Draw(_CHAR_INFO* info, Region rect)
{
    COORD dwBufferSize = { m_dimensions.x, m_dimensions.y };
    COORD dwBufferCoord = { rect.Left, rect.Top };
    SMALL_RECT rcRegion = { m_origin.x + rect.Left, m_origin.y + rect.Top, m_origin.x + rect.Right, m_origin.y + rect.Bottom };
    WriteConsoleOutput(hConsole, info, dwBufferSize, dwBufferCoord, &rcRegion);
}

void WindowsConsole::MoveCursor(Coord pos)
{
    pos = pos + m_origin;
    COORD p = { pos.x, pos.y };
    SetConsoleCursorPosition(hConsole, p);
}

void WindowsConsole::SetCursor(bool enable)
{
    CONSOLE_CURSOR_INFO info;
    info.bVisible = enable;
    info.dwSize = 25;
    SetConsoleCursorInfo(hConsole, &info);
}
