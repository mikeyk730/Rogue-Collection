#pragma once

struct Command
{
    int ch = 0;
    bool m_can_pick_up = false;

    bool is_move() const;
    bool is_run() const;
};

void command();

int com_char();

//Read a command, setting things up according to prefix like devices. Return the command character to be executed.
Command get_command();

void show_count();

void execcom();
