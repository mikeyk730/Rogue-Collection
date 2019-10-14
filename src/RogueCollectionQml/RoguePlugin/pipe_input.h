#pragma once
#ifdef _WIN32
#include <Windows.h>
#endif
#include "replayable_input.h"

struct PipeInput : public ReplayableInput
{
    PipeInput(Environment* current_env, Environment* game_env, const GameConfig& options);
    virtual ~PipeInput() override;

    void RunPipeServer();

#ifdef _WIN32
private:
    HANDLE m_pipe_handle;
#else
    int GetWriteFile() const;

private:
    int fd_[2];
#endif
};
