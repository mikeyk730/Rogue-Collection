//save and restore routines
//save.c      1.32    (A.I. Design)   12/13/84

#include "rogue.h"
#include "game_state.h"
#include "save.h"
#include "io.h"
#include "curses.h"

void do_save(const char* filename)
{
    game->save_game(filename);
    msg("");
}

//save_game: Implement the "save game" command
void save_game()
{
    char savename[20];

    msg("");
    mpos = 0;
    if (in_small_screen_mode())
        addstr("Save file ? ");
    else
        printw("Save file (press enter (\x11\xd9) to default to \"%s\") ? ", game->get_environment("savefile").c_str());
    getinfo(savename, 19);
    if (*savename == 0)
        strcpy(savename, game->get_environment("savefile").c_str());
    do_save(savename);
}

void restore_game(const char *filename)
{
}
