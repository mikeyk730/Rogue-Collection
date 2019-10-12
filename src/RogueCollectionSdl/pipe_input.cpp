#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <exception>
#include "pipe_input.h"

namespace
{
    const int BufferSize = 512;
}

PipeInput::PipeInput(Environment * current_env, Environment * game_env, const GameConfig & options) :
    ReplayableInput(current_env, game_env, options)
{
    if (_pipe(pipe_fd_, BufferSize, _O_BINARY) != 0)
    {
        throw std::runtime_error("Failed to create pipe");
    }

    std::thread t(&PipeInput::RunPipeServer, this);
    t.detach();
}

PipeInput::~PipeInput()
{
    _close(GetReadFd());
    _close(GetWriteFd());
}

bool PipeInput::HandleEvent(const SDL_Event & e)
{
    return false;
}

void PipeInput::RunPipeServer()
{
    char buffer[BufferSize];
    int bytes_read;
        
    while (bytes_read = _read(GetReadFd(), buffer, 1))
    {
        QueueInput(std::string(buffer, buffer + bytes_read));
    }
}

int PipeInput::GetReadFd() const
{
    return pipe_fd_[0];
}

int PipeInput::GetWriteFd() const
{
    return pipe_fd_[1];
}

#endif