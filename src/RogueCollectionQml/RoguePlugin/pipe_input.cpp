#include "pipe_input.h"

#ifdef _WIN32
#include <exception>

namespace
{
    int PipeServerThreadProc(PipeInput* pipe_input)
    {
        pipe_input->RunPipeServer();
        return 0;
    }

    const char* PipeName = "\\\\.\\pipe\\RogueInputPipe";
    const int BufferSize = 512;

    bool s_shutdown = false;
}

PipeInput::PipeInput(Environment * current_env, Environment * game_env, const GameConfig & options) :
    ReplayableInput(current_env, game_env, options)
{
    m_pipe_handle = CreateNamedPipeA(
        PipeName,
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
        1,
        BufferSize,
        BufferSize,
        0,
        NULL);

    if (m_pipe_handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Failed to create pipe");
    }

    std::thread t(PipeServerThreadProc, this);
    t.detach();
}

PipeInput::~PipeInput()
{
    s_shutdown = true;
    DeleteFileA(PipeName);
    CloseHandle(m_pipe_handle);
}

void PipeInput::RunPipeServer()
{
    while (true)
    {
        if (!ConnectNamedPipe(m_pipe_handle, NULL))
        {
            throw std::runtime_error("Failed to connect to pipe");
        }

        if (s_shutdown)
        {
            break;
        }

        char buffer[BufferSize];
        DWORD bytes_read;
        while (ReadFile(m_pipe_handle, buffer, BufferSize, &bytes_read, NULL))
        {
            if (bytes_read > 0)
            {
                QueueInput(std::string(buffer, buffer + bytes_read));
            }
        }

        DisconnectNamedPipe(m_pipe_handle);
    }
}
#else
#include <unistd.h>
#include <thread>

PipeInput::PipeInput(Environment * current_env, Environment * game_env, const GameConfig & options) :
    ReplayableInput(current_env, game_env, options)
{
    pipe(fd_);
    std::thread t(&PipeInput::RunPipeServer, this);
    t.detach();
}

PipeInput::~PipeInput()
{
}

void PipeInput::RunPipeServer()
{
    char buffer[512];
    ssize_t bytes_read;

    while ((bytes_read = read(fd_[0], buffer, 1)) > 0)
    {
        QueueInput(std::string(buffer, buffer + bytes_read));
    }
}

int PipeInput::GetWriteFile() const
{
    return fd_[1];
}

#endif
