#include <QQmlApplicationEngine>
#include <QtQml/QQmlApplicationEngine>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>
#include <QQmlContext>
#include <QStringList>
#include <QScreen>
#include <QFontDatabase>
#include <QtPlugin>
#include <iostream>
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
