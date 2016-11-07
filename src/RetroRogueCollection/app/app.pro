QT += qml quick widgets sql
TARGET = retro-rogue-collection

DESTDIR = $$OUT_PWD/../

HEADERS += \
    fileio.h \
    ../import/utility.h

SOURCES = main.cpp \
    fileio.cpp \
    ../import/utility.cpp \
    ../import/utility_qml.cpp

RESOURCES += qml/resources.qrc

win32:LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/User32.Lib"
