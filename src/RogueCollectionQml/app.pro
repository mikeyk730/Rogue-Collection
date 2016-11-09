TARGET = RogueCollection
QT += qml quick

static {
    QTPLUGIN += RoguePlugin
    LIBS += -LRogue -lrogueplugin
    DEFINES += MDK_STATIC_COMPILE
}

DESTDIR = ./

SOURCES += main.cpp \
    import/utility.cpp \
    import/utility_qml.cpp
RESOURCES += app.qrc

HEADERS += \
    import/utility.h

win32 {
    contains(QT_ARCH, x86_64) {
        LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/User32.Lib"
    } else {
        LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/User32.Lib"
    }
}
