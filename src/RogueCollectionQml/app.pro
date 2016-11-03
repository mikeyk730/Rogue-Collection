TARGET = QRogue
QT += qml quick

DESTDIR = ./

SOURCES += main.cpp \
    import/utility.cpp \
    import/utility_qml.cpp
RESOURCES += app.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/qml/tutorials/extending-qml/chapter6-plugins
INSTALLS += target

HEADERS += \
    import/utility.h

win32:LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/User32.Lib"
