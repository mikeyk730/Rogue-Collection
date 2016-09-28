#include <sstream>
#include <cassert>
#include <cstdio>
#include <conio.h>
#include <thread>
#include "mach_dep.h"
#include "game_state.h"
#include "stream_input.h"
#include "rogue.h"

namespace
{
    int ThreadProc(std::shared_ptr<StreamInput::ThreadData> shared_data)
    {
        for (;;) {
            do {
                std::unique_lock<std::mutex> lock(shared_data->m_mutex);
                if (shared_data->m_stream_empty) {
                    return 0;
                }
            } while (!_kbhit());
            char c = _getch();
            if (c == 'p')
                shared_data->PausePlayback();
            else if (c == 's')
                shared_data->StepPlayback();
            else if (c == 'r')
                shared_data->ResumePlayback();
            else if (c == 'c') {
                shared_data->CancelPlayback();
                break;
            }
        }
        return 0;
    }
}


StreamInput::StreamInput(std::istream& in, int version) :
    m_stream(in),
    m_shared_data(new ThreadData)
{
    if (version < 2) {
        m_version = 'A';
    }
    else {
        read<char>(in, &m_version);
    }

    m_stream.peek();
    if (!m_stream || m_stream.eof()) {
        OnStreamEnd();
    }
    else {
        std::thread t(ThreadProc, m_shared_data);
        t.detach();
    }
}

StreamInput::~StreamInput()
{
}

bool StreamInput::HasMoreInput()
{
    std::unique_lock<std::mutex> lock(m_shared_data->m_mutex);
    return !m_shared_data->m_canceled && !m_shared_data->m_stream_empty;
}

char StreamInput::ReadCharA()
{
    unsigned char f[4];
    m_stream.read((char*)f, 4);
    if (m_stream)
    {
        game->modifiers.m_fast_mode = (f[0] == ON);
        game->modifiers.m_fast_play_enabled = f[1] == ON;
        game->modifiers.m_stop_at_door = f[2] == ON;
        game->modifiers.m_running = f[3] == ON;
    }

    char c = 0;
    m_stream.read(&c, 1);

    if (m_stream && c == 0) //shouldn't happen
    {
        printf("\a");
    }
    return c;
}

char StreamInput::GetNextChar()
{
    std::unique_lock<std::mutex> lock(m_shared_data->m_mutex);
    while (m_shared_data->m_paused && m_shared_data->m_steps == 0) {
        m_shared_data->m_step_cv.wait(lock);
    }
    --m_shared_data->m_steps;

    if (!m_stream || m_shared_data->m_canceled)
        return 0;

    char c = ReadCharA();

    m_stream.peek();
    if (!m_stream || m_stream.eof()) {
        OnStreamEnd();
    }
    return c;
}

std::string StreamInput::ReadStringA()
{
    std::ostringstream ss;

    char c = 0;
    m_stream.read(&c, 1);

    if (m_stream && c != 0) //shouldn't happen
    {
        printf("\a");
    }

    for (int i = 0; m_stream && i < 255; ++i) {
        m_stream.read(&c, 1);
        if (c == 0)
            break;
        ss << c;
    }

    if (ss.str().size() > 150) { //shouldn't happen
        printf("\a");
    }

    return ss.str();
}

std::string StreamInput::ReadStringB()
{
    int size;
    read<int>(m_stream, &size);

    char buf[256];
    memset(buf, 0, 256);
    if (size > 255)
    {
        assert(false);
        size = 255;
    }
    m_stream.read(buf, size);

    return buf;
}

std::string StreamInput::GetNextString(int size)
{
    std::unique_lock<std::mutex> lock(m_shared_data->m_mutex);
    while (m_shared_data->m_paused && m_shared_data->m_steps == 0) {
        m_shared_data->m_step_cv.wait(lock);
    }
    --m_shared_data->m_steps;

    if (!m_stream || m_shared_data->m_canceled)
        return "";

    std::string s;
    if (m_version == 'A')
        s = ReadStringA();
    else
        s = ReadStringB();

    m_stream.peek();
    if (!m_stream || m_stream.eof()) {
        OnStreamEnd();
    }

    return s;
}

void StreamInput::Serialize(std::ostream& out)
{
}

void StreamInput::OnStreamEnd()
{
    m_shared_data->m_stream_empty = true;
    game->m_allow_fast_play = true; //todo: this isn't set if canceled, have game poll for this. this crashes now if save not found
}

void StreamInput::ThreadData::PausePlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = true;
    m_steps = 0;
    m_step_cv.notify_all();
}

void StreamInput::ThreadData::StepPlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_steps++;
    m_step_cv.notify_all();
}

void StreamInput::ThreadData::ResumePlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = false;
    m_steps = 0;
    m_step_cv.notify_all();
}

void StreamInput::ThreadData::CancelPlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_canceled = true;
    m_paused = false;
    m_steps = 0;
    m_step_cv.notify_all();

    game->m_allow_fast_play = true; //todo: remove
}

void StreamInput::ThreadData::OnEmptyStream()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stream_empty = true;
}

//todo:
//serialize enter and esc
//validate serialization
//serialize sizeof
