#pragma once

struct Command
{
    Command();

    int ch = 0;
    bool can_pick_up = true;
    int count = 0;

    int decrement_count();

    bool is_move() const;
    bool is_run() const;
};

void command();

int get_translated_char();

//Read a command, setting things up according to prefix like devices. Return the command character to be executed.
Command get_command();

void show_count(int n);

void execcom();
