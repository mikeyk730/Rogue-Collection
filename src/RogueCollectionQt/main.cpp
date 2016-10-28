#include <memory>
#include <thread>
#include <QApplication>
#include <QFontDatabase>
#include "environment.h"
#include "game_config.h"
#include "qt_rogue.h"
#include "run_game.h"
#include "args.h"

int main(int argc, char *argv[])
{
    Args args = LoadArgs(argc, argv);
    std::shared_ptr<Environment> current_env(new Environment(args));
    InitGameConfig(current_env.get());

    QApplication a(argc, argv);

    auto path = a.applicationDirPath() + "/res/fonts/Px437_IBM_VGA8.ttf";
    int id = QFontDatabase::addApplicationFont(path);
    auto s = QFontDatabase::applicationFontFamilies(id);

    std::unique_ptr<QtRogue> qt_rogue;
    qt_rogue.reset(new QtRogue(0, current_env.get()));

    //start rogue engine on a background thread
    std::thread rogue(RunGame<QtRogue>, "Rogue_PC_1_48.dll", argc, argv, qt_rogue.get());
    rogue.detach(); //todo: how do we want threading to work?

    qt_rogue->show();

    return a.exec();
}

InputInterface::~InputInterface() {}
DisplayInterface::~DisplayInterface() {}
