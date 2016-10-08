#include <memory>
#include "display_interface.h"
#include "coord.h"

struct WindowsConsole : public DisplayInterface
{
    WindowsConsole(Coord origin);
    ~WindowsConsole();

    virtual void SetDimensions(Coord dimensions);
    virtual void Draw(CharInfo* info, bool*);
    virtual void Draw(CharInfo* info, bool*, Region rect);
    virtual void MoveCursor(Coord pos);
    virtual void SetCursor(bool enable);

private:
    Coord m_origin;
    Coord m_dimensions;
    HANDLE hConsole;
};

extern "C"
{
    void init_curses(int r, int c, std::shared_ptr<DisplayInterface> output);
    void shutdown_curses();
    int game_main(int argc, char **argv, char **envp);
}

int main(int argc, char**argv)
{
    std::shared_ptr<DisplayInterface> output(new WindowsConsole({ 0,0 }));
    init_curses(25, 80, output);
    return game_main(argc, argv, 0);
}


DisplayInterface::~DisplayInterface() 
{
}

Coord operator+(const Coord & c1, const Coord & c2)
{
    return{ c1.x + c2.x, c1.y + c2.y };
}

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

void WindowsConsole::Draw(CharInfo* info, bool*)
{
    COORD dwBufferSize = { m_dimensions.x, m_dimensions.y };
    COORD dwBufferCoord = { 0, 0 };
    SMALL_RECT rcRegion = { m_origin.x, m_origin.y, m_origin.x + m_dimensions.x - 1, m_origin.y + m_dimensions.y - 1 };
    WriteConsoleOutput(hConsole, info, dwBufferSize, dwBufferCoord, &rcRegion);
}

void WindowsConsole::Draw(CharInfo* info, bool*, Region rect)
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


