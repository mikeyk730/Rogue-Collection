#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T19:57:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scene
TEMPLATE = app

INCLUDEPATH += $$PWD/../Shared
INCLUDEPATH += $$PWD/../MyCurses


SOURCES += main.cpp\
    args.cpp \
    dos_to_unicode.cpp \
    environment.cpp \
    game_config.cpp \
    key_utility.cpp \
    replayable_input.cpp \
    utility.cpp \
    qt_display.cpp \
    qt_input.cpp \
    qt_rogue.cpp \
    utility_qt.cpp

HEADERS  += \
    args.h \
    dos_to_unicode.h \
    environment.h \
    game_config.h \
    key_utility.h \
    replayable_input.h \
    run_game.h \
    utility.h \
    qt_input.h \
    qt_display.h \
    qt_rogue.h

FORMS    += widget.ui
