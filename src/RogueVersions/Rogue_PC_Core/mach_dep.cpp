//Various installation dependent routines
#include "mach_dep.h"

void exit_game(int status)
{
    throw ExitGame();
}

#ifdef _WIN32
#include <Windows.h>

void sound_beep()
{
    Beep(750, 300);
}

void go_to_sleep(int ms)
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
#else
#include <unistd.h>
void sound_beep()
{
    //todo:mdk
}

void go_to_sleep(int ms)
{
    usleep(ms * 1000);
}

bool is_caps_lock_on()
{
    return false;
}

bool is_num_lock_on()
{
    return false;
}

bool is_scroll_lock_on()
{
    return false;
}
#endif
