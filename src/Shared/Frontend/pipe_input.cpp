#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#define _read read
#endif
#include <thread>
#include <fcntl.h>
#include <exception>
#include "pipe_input.h"

namespace
{
    const int BufferSize = 512;
}

PipeInput::PipeInput(Environment* current_env, Environment* game_env, const GameConfig& options, int fd) :
    ReplayableInput(current_env, game_env, options),
    pipe_fd_(fd)
{
    std::thread t(&PipeInput::RunPipeServer, this);
    t.detach();
}

PipeInput::~PipeInput()
{
}

void PipeInput::RunPipeServer()
{
    char buffer[BufferSize];
    int bytes_read;

    while ((bytes_read = _read(pipe_fd_, buffer, 1)))
    {
        if (bytes_read == 1 && buffer[0] == EOF) {
            break;
        }

        QueueInput(std::string(buffer, buffer + bytes_read));
    }
}
