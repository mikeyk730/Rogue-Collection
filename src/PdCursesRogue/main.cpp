#include <memory>
#include "pdc_keyboard_input.h"
#include "RogueCore/main.h"
#include "RogueCore/output_interface.h"
#include <input_interface.h>
#include <display_interface.h>

extern "C"
{
    __declspec(dllexport) int rogue_main(int argc, char **argv);
    __declspec(dllexport) void init_game(struct DisplayInterface* screen, struct InputInterface* input);
    void init_curses(DisplayInterface* screen, InputInterface* input);
}

namespace
{
    InputInterface* s_input;
}

std::shared_ptr<OutputInterface> CreateCursesOutput();

void init_game(DisplayInterface* screen, InputInterface* input)
{
    s_input = input;
    init_curses(screen, input);
}

int rogue_main(int argc, char **argv)
{
    std::shared_ptr<OutputInterface> output(CreateCursesOutput());
    std::shared_ptr<InputInterfaceEx> input(new ConsoleKeyboardInput(s_input));

    game_main(argc, argv, output, input);

    return 0;
}
