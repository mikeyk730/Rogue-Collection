#include <conio.h>
#include <Windows.h>
#include "console_keyboard_input.h"
#include "rogue.h"
#include "io.h"

#define C_LEFT    0x4b
#define C_RIGHT   0x4d
#define C_UP      0x48
#define C_DOWN    0x50
#define C_HOME    0x47
#define C_PGUP    0x49
#define C_END     0x4f
#define C_PGDN    0x51
#define C_ESCAPE  0x1b
#define C_INS     0x52
#define C_DEL     0x53
#define C_F1      0x3b
#define C_F2      0x3c
#define C_F3      0x3d
#define C_F4      0x3e
#define C_F5      0x3f
#define C_F6      0x40
#define C_F7      0x41
#define C_F8      0x42
#define C_F9      0x43
#define C_F10     0x44
#define ALT_F9    0x70

//Table for IBM extended key translation
static struct xlate
{
    byte keycode, keyis; //todo: need to translate all of these
} xtab[] =
{
    C_HOME,  'y',
    C_UP,    'k',
    C_PGUP,  'u',
    C_LEFT,  'h',
    C_RIGHT, 'l',
    C_END,   'b',
    C_DOWN,  'j',
    C_PGDN,  'n',
    C_INS,   '>',
    C_DEL,   's',
    C_F1,    '?',
    C_F2,    '/',
    C_F3,    'a',
    C_F4,    CTRL('R'),
    C_F5,    'c',
    C_F6,    'D',
    C_F7,    'i',
    C_F8,    '^',
    C_F9,    CTRL('F'),
    C_F10,   '!',
    ALT_F9,  'F'
};

bool is_shift_pressed()
{
    return (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || 
        (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
}

int is_direction_key(int key)
{
    return key == C_HOME
        || key == C_UP
        || key == C_PGUP
        || key == C_LEFT
        || key == C_RIGHT
        || key == C_END
        || key == C_DOWN
        || key == C_PGDN;
}

int getkey()
{
    struct xlate *x;
    int key = _getch();
    if (key != 0 && key != 0xE0) return key;

    key = _getch();
    for (x = xtab; x < xtab + (sizeof xtab) / sizeof *xtab; x++)
    {
        if (key == x->keycode)
        {
            if (is_shift_pressed() && is_direction_key(key))
                return toupper(x->keyis);
            return x->keyis;
        }
    }
    return 0;
}

byte readchar_impl() {
    //while there are no characters in the type ahead buffer update the status line at the bottom of the screen
    do
        handle_key_state();
    while (!_kbhit()); //Rogue spends a lot of time here
                       //Now read a character and translate it if it appears in the translation table
    return getkey();
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
    return TRUE;
}



ConsoleKeyboardInput::ConsoleKeyboardInput()
{
    //mdk: don't exit on Ctrl+C
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
}

bool ConsoleKeyboardInput::HasMoreInput()
{
    return true;
}

char ConsoleKeyboardInput::GetNextChar() {
    return readchar_impl();
}

std::string ConsoleKeyboardInput::GetNextString(int size) {
    char buf[512];
    getinfo_impl(buf, size - 1);
    return buf;
}

bool ConsoleKeyboardInput::IsCapsLockOn()
{
    return LOBYTE(GetKeyState(VK_CAPITAL)) != 0;
}

bool ConsoleKeyboardInput::IsNumLockOn()
{
    return LOBYTE(GetKeyState(VK_NUMLOCK)) != 0;
}

bool ConsoleKeyboardInput::IsScrollLockOn()
{
    return LOBYTE(GetKeyState(VK_SCROLL)) != 0;
}

void ConsoleKeyboardInput::Serialize(std::ostream& out)
{ }
