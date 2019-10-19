#include <QQmlApplicationEngine>
#include <QtQml/QQmlApplicationEngine>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>
#include <QQmlContext>
#include <QStringList>
#include <QScreen>
#include <QFontDatabase>
#include <QDebug>
#include <QtPlugin>
#include "fileio.h"
#include "start_process.h"
#include "../RoguePlugin/utility_qml.h"

#ifdef MDK_STATIC_COMPILE
Q_IMPORT_PLUGIN(RoguePlugin)
#endif

int RunMain(int argc, char** argv);

int main(int argc, char** argv)
{
    return StartProcess(RunMain, argc, argv);
}

int RunMain(int argc, char *argv[])
{
#ifdef MDK_STATIC_COMPILE
    Q_INIT_RESOURCE(rogue_resources);
#endif

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    FileIO fileIO;

    // Manage command line arguments from the cpp side
    QStringList args = app.arguments();
    if (args.contains("-h") || args.contains("--help")) {
        qDebug() << "Usage: " + args.at(0) + " [flags] [savefile|name]";
        qDebug() << "  -h|--help           Print this help.";
        qDebug() << "  -o|--optfile <file> Use the given Rogue option file";
        qDebug() << "  -n|--small-screen   Run with the smallest screen that the game supports";
        qDebug() << "  -p|--paused         Start replay paused";
        qDebug() << "  --pause-at <n>      Pause replay with <n> steps remaining";
        qDebug() << "  --default-settings  Run with the default profile";
        qDebug() << "  --profile <prof>    Run with the given profile.";
        qDebug() << "  --rogomatic         Spawn Rog-O-Matic to play the game.";
        //qDebug() << "  --fullscreen        Run in fullscreen.";
        qDebug() << "  --verbose           Print additional information such as profiles and settings.";

        return 0;
    }

    engine.rootContext()->setContextProperty("fileIO", &fileIO);
    engine.rootContext()->setContextProperty("devicePixelRatio", app.devicePixelRatio());

    int scale = 1;
    if (app.primaryScreen()->geometry().width() >= 3000)
        scale = 2;
    engine.rootContext()->setContextProperty("defaultScale", scale);

    auto path = app.applicationDirPath() + "/res/fonts/Px437_IBM_VGA8.ttf";
    QFontDatabase::addApplicationFont(path);

    try {
        engine.load(QUrl("qrc:///main.qml"));
        if (engine.rootObjects().isEmpty()) {
            return EXIT_FAILURE;
        }

        // Quit the application when the engine closes.
        QObject::connect((QObject*) &engine, SIGNAL(quit()), (QObject*) &app, SLOT(quit()));

        return app.exec();
    }
    catch (std::runtime_error& e)
    {
        DisplayMessage("Error", "Fatal Error", e.what());
        return EXIT_FAILURE;
    }
}
