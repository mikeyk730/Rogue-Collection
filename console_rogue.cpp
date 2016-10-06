#include <memory>
#include "windows_console.h"
#include "main.h"

int main(int argc, char **argv)
{
    std::unique_ptr<ScreenInterface> screen(new WindowsConsole({ 0, 0 }));
    game_main(argc, argv, screen.get());
}
