#pragma once
#ifdef _WIN32
#include <SDL.h>
#include "replayable_input.h"

struct PipeInput : public ReplayableInput
{
    PipeInput(Environment* current_env, Environment* game_env, const GameConfig& options, int fd);
    ~PipeInput();

    void RunPipeServer();

private:
    int pipe_fd_;
};
#endif
