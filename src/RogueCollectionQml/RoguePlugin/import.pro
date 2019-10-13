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
    dos_to_unicode.h \
    environment.h \
    game_config.h \
    replayable_input.h \
    pipe_input.h \
    run_game.h \
    utility.h \
    key_utility.h \
    qrogue_display.h \
    qrogue_input.h \
    tile_provider.h \
    colors.h

SOURCES += \
    plugin.cpp \
    qrogue.cpp \
    dos_to_unicode.cpp \
    environment.cpp \
    game_config.cpp \
    replayable_input.cpp \
    pipe_input.cpp \
    utility.cpp \
    key_utility.cpp \
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
}

OTHER_FILES += qmldir

# Copy the qmldir file to the same folder as the plugin binary
cpqmldir.files = qmldir
cpqmldir.path = $$DESTDIR
COPIES += cpqmldir

RESOURCES += rogue_resources.qrc
