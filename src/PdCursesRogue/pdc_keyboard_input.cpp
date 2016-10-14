#include <conio.h>
#include <Windows.h>
#undef MOUSE_MOVED
#include <curses.h>
#include "pdc_keyboard_input.h"
#include "RogueCore/rogue.h"
#include "RogueCore/io.h"
#include "RogueCore/game_state.h"
#include "RogueCore/output_shim.h"
#include "RogueCore/mach_dep.h"

////Table for IBM extended key translation
//static struct xlate
//{
//    int keycode, keyis; //todo: need to translate all of these
//} xtab[] =
//{
//    KEY_HOME,  'y',
//    KEY_UP,    'k',
//    KEY_PPAGE, 'u',
//    KEY_LEFT,  'h',
//    KEY_RIGHT, 'l',
//    KEY_END,   'b',
//    KEY_DOWN,  'j',
//    KEY_NPAGE, 'n',
//    KEY_IC,    '>',
//    KEY_DC,    's',
///*    KEY_F1,    '?',
//    KEY_F2,    '/',
//    KEY_F3,    'a',
//    KEY_F4,    CTRL('R'),
//    KEY_F5,    'c',
//    KEY_F6,    'D',
//    KEY_F7,    'i',
//    KEY_F8,    '^',
//    KEY_F9,    CTRL('F'),
//    KEY_F10,   '!',
//    ALT_F9,  'F'*/
//};
//
//bool is_shift_pressed()
//{
//    return (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || 
//        (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
//}
//
//int is_direction_key(int key)
//{
//    return key == KEY_HOME
//        || key == KEY_UP
//        || key == KEY_PPAGE
//        || key == KEY_LEFT
//        || key == KEY_RIGHT
//        || key == KEY_END
//        || key == KEY_DOWN
//        || key == KEY_NPAGE;
//}
//
//int getkey()
//{
//    struct xlate *x;
//    int key = getch();
//    for (x = xtab; x < xtab + (sizeof xtab) / sizeof *xtab; x++)
//    {
//        if (key == x->keycode)
//        {
//            if (is_shift_pressed() && is_direction_key(key))
//                return toupper(x->keyis);
//            return x->keyis;
//        }
//    }
//    return key;
//}
//
//void backspace()
//{
//    int x, y;
//    game->screen().getrc(&x, &y);
//    if (--y < 0) y = 0;
//    game->screen().move(x, y);
//    game->screen().add_text(' ');
//    game->screen().move(x, y);
//}
//
////This routine reads information from the keyboard. It should do all the strange processing that is needed to retrieve sensible data from the user
//int getinfo_impl(char *str, int size)
//{
//    char *retstr, ch;
//    int readcnt = 0;
//    bool wason;
//    int ret = 1;
//
//    retstr = str;
//    *str = 0;
//    wason = game->screen().cursor(true);
//    while (ret == 1) switch (ch = getkey())
//    {
//    case ESCAPE:
//        while (str != retstr) { backspace(); readcnt--; str--; }
//        ret = *str = ESCAPE;
//        str[1] = 0;
//        game->screen().cursor(wason);
//        break;
//    case '\b':
//        if (str != retstr) { backspace(); readcnt--; str--; }
//        break;
//    default:
//        if (readcnt >= size) { /*sound_beep();*/ break; }
//        readcnt++;
//        game->screen().add_text(ch);
//        *str++ = ch;
//        if ((ch & 0x80) == 0) break;
//    case '\n':
//    case '\r':
//        *str = 0;
//        game->screen().cursor(wason);
//        ret = ch;
//        break;
//    }
//    return ret;
//}
//
//
//byte readchar_impl() {
//    //while there are no characters in the type ahead buffer update the status line at the bottom of the screen
//    //do
//    //    handle_key_state();
//    //while (!curses_check_key()); //Rogue spends a lot of time here
//    return getkey();
//}
//
//BOOL CtrlHandler(DWORD fdwCtrlType)
//{
//    return TRUE;
//}

ConsoleKeyboardInput::ConsoleKeyboardInput(InputInterface* i)
    :impl(i)
{
}

bool ConsoleKeyboardInput::HasMoreInput()
{
    return true;
}

char ConsoleKeyboardInput::GetNextChar() {
    return impl->GetChar(true);
}

std::string ConsoleKeyboardInput::GetNextString(int size) {
    return impl->GetString(size);
}

void ConsoleKeyboardInput::Serialize(std::ostream& out)
{ }
