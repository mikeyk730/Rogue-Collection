#include <QQmlApplicationEngine>
#include <QtQml/QQmlApplicationEngine>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>
#include <QQmlContext>
#include <QStringList>
#include <QFontDatabase>
#include <QDebug>
#include <QtPlugin>
#include "fileio.h"
#include "../RoguePlugin/utility.h"

#ifdef MDK_STATIC_COMPILE
Q_IMPORT_PLUGIN(RoguePlugin)
#endif

int main(int argc, char *argv[])
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
        qDebug() << "Usage: " + args.at(0) + " [--default-settings] [--fullscreen] [--profile <prof>] [-h|--help] [--verbose]";
        qDebug() << "  --default-settings  Run with the default settings";
        qDebug() << "  --fullscreen        Run in fullscreen.";
        qDebug() << "  --profile <prof>    Run with the given profile.";
        qDebug() << "  -h|--help           Print this help.";
        qDebug() << "  --verbose           Print additional information such as profiles and settings.";
        return 0;
    }

    engine.rootContext()->setContextProperty("fileIO", &fileIO);
    engine.rootContext()->setContextProperty("devicePixelRatio", app.devicePixelRatio());

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
