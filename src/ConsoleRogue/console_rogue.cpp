#include <memory>
#include "windows_console.h"
#include "console_keyboard_input.h"

int main(int argc, char **argv)
{
    std::shared_ptr<DisplayInterface> display(new WindowsConsole({ 0, 0 }));
    auto output = CreateScreenOutput(display);

    std::shared_ptr<InputInterface> input(new ConsoleKeyboardInput());

    game_main(argc, argv, output, input);
}
