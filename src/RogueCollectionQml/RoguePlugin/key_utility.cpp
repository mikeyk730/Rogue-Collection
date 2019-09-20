#include "key_utility.h"

bool IsLetterKey(unsigned int keycode)
{
    return (keycode >= 'a' && keycode <= 'z');
}

bool IsDirectionKey(unsigned int keycode)
{
    switch (keycode)
    {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case 'y':
    case 'u':
    case 'b':
    case 'n':
        return true;
    }
    return false;
}

#ifdef WIN32
#include <Windows.h>
bool IsCapsLockOn()
{
    return LOBYTE(GetKeyState(VK_CAPITAL)) != 0;
}
#else
bool IsCapsLockOn()
{
    return false; //todo:mdk
}
#endif
