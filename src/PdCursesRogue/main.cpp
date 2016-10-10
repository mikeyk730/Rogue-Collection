#include <memory>
#include "pdc_keyboard_input.h"
#include "RogueCore/main.h"
#include "RogueCore/output_interface.h"

std::shared_ptr<OutputInterface> CreateCursesOutput();

int main(int argc, char **argv)
{
    std::shared_ptr<OutputInterface> output(CreateCursesOutput());
    std::shared_ptr<InputInterface> input(new ConsoleKeyboardInput());

    game_main(argc, argv, output, input);
}
