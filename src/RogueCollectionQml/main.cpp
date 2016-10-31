#include <QtQuick/QQuickView>
#include <QGuiApplication>
#include <QFontDatabase>
#include <iostream>

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    auto path = a.applicationDirPath() + "/res/fonts/Px437_IBM_VGA8.ttf";
    QFontDatabase::addApplicationFont(path);

    QQuickView view;
    view.setResizeMode(QQuickView::SizeViewToRootObject);
    view.setSource(QUrl("qrc:///app.qml"));
    view.show();
    return a.exec();
}
