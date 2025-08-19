TEMPLATE = lib
CONFIG += plugin
QT += qml quick multimedia

static {
    URI = RoguePlugin
    QMAKE_MOC_OPTIONS += -Muri=$$URI
}

linux {
    QMAKE_CXXFLAGS += -Wall -Werror -pedantic
}

DESTDIR = $$OUT_PWD
TARGET = $$qtLibraryTarget(rogueplugin)

INCLUDEPATH += $$PWD/../../Shared
INCLUDEPATH += $$PWD/../../Shared/Frontend
INCLUDEPATH += $$PWD/../../MyCurses

HEADERS += \
    plugin.h \
    qrogue.h \
    utility_qml.h \
    qrogue_display.h \
    qrogue_input.h \
    tile_provider.h \
    colors.h

SOURCES += \
    plugin.cpp \
    qrogue.cpp \
    game_config.cpp \
    utility_qml.cpp \
    qrogue_display.cpp \
    qrogue_input.cpp \
    text_provider.cpp \
    tile_provider.cpp \
    font_provider.cpp \
    colors.cpp

win32 {
    DEFINES += "WINVER=0x0500"
    contains(QT_ARCH, x86_64) {
        LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64/User32.Lib"
    } else {
        LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x86/User32.Lib"
    }

    CONFIG( debug, debug|release ) {
        LIBS += "$$PWD/../../../bin/Win32/Debug/Frontend.lib"
    }
    else {
        LIBS += "$$PWD/../../../bin/Win32/Release/Frontend.lib"
    }
}
linux {
    CONFIG( debug, debug|release ) {
        LIBS += "$$PWD/../../../build/debug/lib-shared-frontend.a"
    }
    else {
        LIBS += "$$PWD/../../../build/release/lib-shared-frontend.a"
    }
}

OTHER_FILES += qmldir

# Copy the qmldir file to the same folder as the plugin binary
cpqmldir.files = qmldir
cpqmldir.path = $$DESTDIR
COPIES += cpqmldir

RESOURCES += rogue_resources.qrc
