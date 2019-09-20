#pragma once

struct DisplayInterface;
struct InputInterface;

extern "C"
{
    void init_curses(DisplayInterface* screen, InputInterface* input, int lines, int cols);
    void shutdow_curses();
    void play_sound(const char* id);
    bool has_typeahead();
}
