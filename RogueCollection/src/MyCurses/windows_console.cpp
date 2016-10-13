#include <Windows.h>
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

void WindowsConsole::UpdateRegion(CursesDataType** buf, Region rect)
{
    rect = { 0,0,79,24 };
    CHAR_INFO* info = new CHAR_INFO[rect.Width()*rect.Height()];
    for (int r = rect.Top; r <= rect.Bottom; ++r)
    {
        for (int c = rect.Left; c <= rect.Right; ++c) {
            auto ch = buf[rect.Top + r][rect.Left + c];
            int text = ch & 0xffff;
            int color = (ch >> 24) & 0xff;
            info[r*rect.Width() + c].Char.AsciiChar = text;
            info[r*rect.Width() + c].Attributes = color ? color : 0x07;
        }
    }

    COORD dwBufferSize = { rect.Width(), rect.Height() };
    COORD dwBufferCoord = { 0, 0 };
    SMALL_RECT rcRegion = { m_origin.x + rect.Left, m_origin.y + rect.Top, m_origin.x + rect.Right, m_origin.y + rect.Bottom };
    WriteConsoleOutput(hConsole, info, dwBufferSize, dwBufferCoord, &rcRegion);
}

void WindowsConsole::MoveCursor(Coord pos)
{
    pos.x = pos.x + m_origin.x;
    pos.y = pos.y + m_origin.y;
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
