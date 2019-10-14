#include <QtQuick/QQuickView>
#include <QtWidgets/QApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QtPlugin>
#include <QQmlContext>
#include <QScreen>
#include "../RoguePlugin/utility_qml.h"

#ifdef MDK_STATIC_COMPILE
Q_IMPORT_PLUGIN(RoguePlugin)
#endif

int main(int argc, char *argv[])
{
#ifdef MDK_STATIC_COMPILE
    Q_INIT_RESOURCE(rogue_resources);
#endif

    QApplication a(argc, argv);

    auto path = a.applicationDirPath() + "/res/fonts/Px437_IBM_VGA8.ttf";
    QFontDatabase::addApplicationFont(path);

    try {
        QQmlApplicationEngine engine;

        int scale = 1;
        if (a.primaryScreen()->geometry().width() >= 3000)
            scale = 2;
        engine.rootContext()->setContextProperty("defaultScale", scale);

        engine.load(QUrl("qrc:///app.qml"));
        QObject* topLevel = engine.rootObjects().value(0);
        QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);
        window->show();

        return a.exec();
    }
    catch (std::runtime_error& e)
    {
        DisplayMessage("Error", "Fatal Error", e.what());
        return 1;
    }
}
