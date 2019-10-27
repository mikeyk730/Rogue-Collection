#pragma once

struct DisplayInterface;
struct InputInterface;

#ifdef __cplusplus
extern "C"
{
#endif
    void init_curses(struct DisplayInterface* screen, struct InputInterface* input, int lines, int cols);
    void shutdow_curses();
    void play_sound(const char* id);
    int has_typeahead();
#ifdef __cplusplus
}
#endif
