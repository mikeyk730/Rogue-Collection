TEMPLATE = lib
CONFIG += plugin
QT += qml quick

DESTDIR = ../Rogue
TARGET = $$qtLibraryTarget(rogueplugin)

DEFINES += "WINVER=0x0500"
win32:LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/User32.Lib"

HEADERS += \
    plugin.h \
    qrogue.h \
    args.h \
    dos_to_unicode.h \
    environment.h \
    game_config.h \
    replayable_input.h \
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
    args.cpp \
    dos_to_unicode.cpp \
    environment.cpp \
    game_config.cpp \
    replayable_input.cpp \
    utility.cpp \
    key_utility.cpp \
    utility_qml.cpp \
    qrogue_display.cpp \
    qrogue_input.cpp \
    text_provider.cpp \
    tile_provider.cpp \
    font_provider.cpp \
    colors.cpp

INCLUDEPATH += $$PWD/../../Shared
INCLUDEPATH += $$PWD/../../MyCurses

DESTPATH=$$[QT_INSTALL_EXAMPLES]/qml/tutorials/extending-qml/chapter6-plugins/Rogue

target.path=$$DESTPATH
qmldir.files=$$PWD/qmldir
qmldir.path=$$DESTPATH
INSTALLS += target qmldir

CONFIG += install_ok  # Do not cargo-cult this!

OTHER_FILES += qmldir

# Copy the qmldir file to the same folder as the plugin binary
cpqmldir.files = qmldir
cpqmldir.path = $$DESTDIR
COPIES += cpqmldir

RESOURCES += \
    resources.qrc

DISTFILES +=
