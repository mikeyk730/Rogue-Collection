#include <QQmlApplicationEngine>
#include <QtQml/QQmlApplicationEngine>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>
#include <QQmlContext>
#include <QStringList>
#include <QFontDatabase>
#include <QDebug>
#include "fileio.h"
#include "../import/utility.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    FileIO fileIO;

    // Manage command line arguments from the cpp side
    QStringList args = app.arguments();
    if (args.contains("-h") || args.contains("--help")) {
        qDebug() << "Usage: " + args.at(0) + " [--default-settings] [-p|--profile <prof>] [--fullscreen] [-h|--help]";
        qDebug() << "  --default-settings  Run cool-retro-term with the default settings";
        qDebug() << "  --fullscreen        Run cool-retro-term in fullscreen.";
        qDebug() << "  -p|--profile <prof> Run cool-retro-term with the given profile.";
        qDebug() << "  -h|--help           Print this help.";
        qDebug() << "  --verbose           Print additional information such as profiles and settings.";
        return 0;
    }

    engine.rootContext()->setContextProperty("fileIO", &fileIO);
    engine.rootContext()->setContextProperty("devicePixelRatio", app.devicePixelRatio());

    auto path = app.applicationDirPath() + "/res/fonts/Px437_IBM_VGA8.ttf";
    QFontDatabase::addApplicationFont(path);

    try {
        engine.load(QUrl(QStringLiteral ("qrc:/main.qml")));
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
