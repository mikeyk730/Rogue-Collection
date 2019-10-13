#pragma once
#include "replayable_input.h"

struct PipeInput : public ReplayableInput
{
    PipeInput(Environment* current_env, Environment* game_env, const GameConfig& options, int pipe_fd);
    ~PipeInput();

    void RunPipeServer();

private:
    int pipe_fd_;
};
