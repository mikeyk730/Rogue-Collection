#include <memory>
#include "windows_console.h"
#include "console_keyboard_input.h"
#include "main.h"

int main(int argc, char **argv)
{
    std::unique_ptr<ScreenInterface> screen(new WindowsConsole({ 0, 0 }));
    std::unique_ptr<InputInterface> input(new ConsoleKeyboardInput());

    game_main(argc, argv, std::move(screen), std::move(input));
}
