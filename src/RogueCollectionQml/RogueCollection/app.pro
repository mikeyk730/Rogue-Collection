QT += widgets qml quick multimedia
linux {
    TARGET = rogue-collection
    QMAKE_LFLAGS += -fno-pic -no-pie
    QMAKE_CXXFLAGS += -Wall -Werror -pedantic
}
win32 {
    TARGET = RogueCollection
}

static {
    QTPLUGIN += RoguePlugin
    LIBS += -L../RoguePlugin -lrogueplugin
    DEFINES += MDK_STATIC_COMPILE
}

DESTDIR = $$OUT_PWD/../

INCLUDEPATH += $$PWD/../../Shared/Frontend

HEADERS += \
    ../RoguePlugin/utility_qml.h

SOURCES = main.cpp \
    ../RoguePlugin/utility_qml.cpp

RESOURCES += app.qrc

win32 {
    DEFINES += "WINVER=0x0500"
    contains(QT_ARCH, x86_64) {
        LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64/User32.Lib"
    } else {
        LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x86/User32.Lib"
    }

    CONFIG( debug, debug|release ) {
        LIBS += "$$PWD/../../../bin/Win32/Debug/Frontend.lib"
    } else {
        LIBS += "$$PWD/../../../bin/Win32/Release/Frontend.lib"
    }
}
linux {
    CONFIG( debug, debug|release ) {
        LIBS += "$$PWD/../../../build/debug/lib-shared-frontend.a"
    } else {
        LIBS += "$$PWD/../../../build/release/lib-shared-frontend.a"
    }
}
