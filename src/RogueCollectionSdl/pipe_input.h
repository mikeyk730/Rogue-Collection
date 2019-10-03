#pragma once
#ifdef _WIN32
#include <Windows.h>
#include <SDL.h>
#include "replayable_input.h"

struct PipeInput : public ReplayableInput
{
    PipeInput(Environment* current_env, Environment* game_env, const GameConfig& options);
    ~PipeInput();

    virtual bool HandleEvent(const SDL_Event& e) override;

    void RunPipeServer();

private:
    HANDLE m_pipe_handle;
};
#endif
