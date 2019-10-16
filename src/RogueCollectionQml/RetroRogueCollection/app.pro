QT += qml quick widgets sql multimedia

linux {
    TARGET = retro-rogue-collection
    QMAKE_LFLAGS += -fno-pic -no-pie
    QMAKE_CXXFLAGS += -Wall -Werror -pedantic
}
win32 {
    TARGET = RetroRogueCollection
}

static {
    QTPLUGIN += RoguePlugin
    LIBS += -L../RoguePlugin -lrogueplugin
    DEFINES += MDK_STATIC_COMPILE
}

DESTDIR = $$OUT_PWD/../

INCLUDEPATH += $$PWD/../../Shared/Frontend

HEADERS += \
    fileio.h \
    ../RoguePlugin/utility_qml.h

SOURCES = main.cpp \
    fileio.cpp \
    ../RoguePlugin/utility_qml.cpp

RESOURCES += qml/resources.qrc

win32 {
    DEFINES += "WINVER=0x0500"
    contains(QT_ARCH, x86_64) {
        LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/User32.Lib"
    } else {
        LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/User32.Lib"
    }

    debug {
        LIBS += "$$PWD/../../../bin/Win32/Debug/Frontend.lib"
    }
    release {
        LIBS += "$$PWD/../../../bin/Win32/Release/Frontend.lib"
    }
}
linux {
    debug {
        LIBS += "$$PWD/../../../build/debug/lib-shared-frontend.a"
    }
    release {
        LIBS += "$$PWD/../../../build/release/lib-shared-frontend.a"
    }
}
