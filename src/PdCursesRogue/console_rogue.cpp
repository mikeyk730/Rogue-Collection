#include <memory>
#include "windows_console.h"
#include "pdc_keyboard_input.h"
#include "RogueCore/main.h"

int main(int argc, char **argv)
{
    std::shared_ptr<DisplayInterface> output(new WindowsConsole({ 0, 0 }));
    std::shared_ptr<InputInterface> input(new ConsoleKeyboardInput());

    game_main(argc, argv, output, input);
}
