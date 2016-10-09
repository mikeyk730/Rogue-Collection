//save and restore routines
//save.c      1.32    (A.I. Design)   12/13/84

#include "rogue.h"
#include "game_state.h"
#include "save.h"
#include "io.h"
#include "output_shim.h"

void do_save(const char* filename)
{
    game->save_game(filename);
    msg("");
}

//do_save_game: Implement the "save game" command
bool do_save_game()
{
    char savename[20];

    msg("");
    reset_msg_position();
    if (in_small_screen_mode())
        game->screen().addstr("Save file? ");
    else
        game->screen().printw("Save file (press enter (\x11\xd9) to default to \"%s\") ? ", game->get_environment("savefile").c_str());
    getinfo(savename, 19);
    if (*savename == 0)
        strcpy(savename, game->get_environment("savefile").c_str());
    do_save(savename);

    return false;
}

void restore_game(const char *filename)
{
}
