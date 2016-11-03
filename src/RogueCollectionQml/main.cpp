#include <QtQuick/QQuickView>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <iostream>
#include "import/utility.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    auto path = a.applicationDirPath() + "/res/fonts/Px437_IBM_VGA8.ttf";
    QFontDatabase::addApplicationFont(path);

    try {
        QQmlApplicationEngine engine(QUrl("qrc:///app.qml"));
        QObject* topLevel = engine.rootObjects().value(0);
        QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);

        window->setWidth(640*2);
        window->setHeight(400*2);
        window->show();

        return a.exec();
    }
    catch (std::runtime_error& e)
    {
        DisplayMessage("Error", "Fatal Error", e.what());
        return 1;
    }
}
