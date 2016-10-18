#include <memory>
#include "curses_input.h"
#include "RogueCore/main.h"
#include "RogueCore/output_interface.h"
#include <input_interface.h>
#include "../Shared/display_interface.h"

extern "C"
{
    __declspec(dllexport) int rogue_main(int argc, char **argv);
    __declspec(dllexport) void init_game(struct DisplayInterface* screen, struct InputInterface* input, int lines, int cols);
    void init_curses(DisplayInterface* screen, InputInterface* input, int lines, int cols);

    std::shared_ptr<InputInterfaceEx> s_input;
}

std::shared_ptr<OutputInterface> CreateCursesOutput();

void init_game(DisplayInterface* screen, InputInterface* input, int lines, int cols)
{
    s_input.reset(new ConsoleKeyboardInput(input));
    init_curses(screen, input, lines, cols);
}

int rogue_main(int argc, char **argv)
{
    std::shared_ptr<OutputInterface> output(CreateCursesOutput());

    game_main(argc, argv, output, s_input);

    return 0;
}
