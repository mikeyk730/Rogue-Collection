TARGET = QRogue
QT += qml quick

DESTDIR = ./

SOURCES += main.cpp
RESOURCES += app.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/qml/tutorials/extending-qml/chapter6-plugins
INSTALLS += target

HEADERS +=

