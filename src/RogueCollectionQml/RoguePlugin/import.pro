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
    game_config.h \
    pipe_input.h \
    run_game.h \
    utility.h \
    qrogue_display.h \
    qrogue_input.h \
    tile_provider.h \
    colors.h

SOURCES += \
    plugin.cpp \
    qrogue.cpp \
    game_config.cpp \
    pipe_input.cpp \
    utility.cpp \
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

OTHER_FILES += qmldir

# Copy the qmldir file to the same folder as the plugin binary
cpqmldir.files = qmldir
cpqmldir.path = $$DESTDIR
COPIES += cpqmldir

RESOURCES += rogue_resources.qrc
