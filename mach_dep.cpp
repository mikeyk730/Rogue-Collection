//Various installation dependent routines
#include <Windows.h>
#include "mach_dep.h"
#include "game_state.h"

void beep()
{
    if (!game->in_replay())
        Beep(750, 300);
}

void tick_pause()
{
    if(!game->in_replay())
        Sleep(50);
}
