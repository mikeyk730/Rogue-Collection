QT += qml quick widgets sql
TARGET = RetroRogueCollection

static {
    QTPLUGIN += RoguePlugin
    LIBS += -L../Rogue -lrogueplugin
    DEFINES += MDK_STATIC_COMPILE
}

DESTDIR = $$OUT_PWD/../

HEADERS += \
    fileio.h \
    ../import/utility.h

SOURCES = main.cpp \
    fileio.cpp \
    ../import/utility.cpp \
    ../import/utility_qml.cpp

RESOURCES += qml/resources.qrc

win32 {
    DEFINES += "WINVER=0x0500"
    contains(QT_ARCH, x86_64) {
        LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/User32.Lib"
    } else {
        LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/User32.Lib"
    }
}
