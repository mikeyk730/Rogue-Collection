//Various installation dependent routines
#include "mach_dep.h"

#ifdef _WIN32
#include <Windows.h>

void sound_beep()
{
    Beep(750, 300);
}

void sleep(int ms)
{
    Sleep(ms);
}

bool is_caps_lock_on()
{
    return LOBYTE(GetKeyState(VK_CAPITAL)) != 0;
}

bool is_num_lock_on()
{
    return LOBYTE(GetKeyState(VK_NUMLOCK)) != 0;
}

bool is_scroll_lock_on()
{
    return LOBYTE(GetKeyState(VK_SCROLL)) != 0;
}
#endif
