#include <sstream>
#include <cstring>
#include <cassert>
#include <cstdio>
#ifdef _WIN32
#include <conio.h>
#endif
#include <thread>
#include <coord.h>
#include "io.h"
#include "stream_input.h"
#include "rogue.h"
#include "mach_dep.h"

namespace
{
    int ThreadProc(std::shared_ptr<StreamInput::ThreadData> shared_data)
    {
#ifdef _WIN32 //todo:mdk support linux
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
#endif
        return 0;
    }
}


StreamInput::StreamInput(std::unique_ptr<std::istream> in, int version, bool start_paused) :
    m_stream(std::move(in)),
    m_shared_data(new ThreadData)
{
    read(*m_stream, &m_version);
    if (m_version < 'C' || m_version > 'D') {
        throw std::runtime_error("Unsupported save version: " + m_version);
    }

    m_shared_data->m_paused = start_paused;
    std::thread t(ThreadProc, m_shared_data);
    t.detach();
}

StreamInput::~StreamInput()
{
}

bool StreamInput::HasMoreInput()
{
    std::unique_lock<std::mutex> lock(m_shared_data->m_mutex);
    return !m_shared_data->m_canceled && !m_shared_data->m_stream_empty;
}

char StreamInput::ReadChar()
{
    unsigned char info[2];
    m_stream->read((char*)info, 2);

    char c = 0;

    if (*m_stream)
    {
        byte fast_play(info[0]);
        c = info[1];

        NotifyFastPlayChanged(fast_play ? true : false);
        if (*m_stream && c == 0)
        {
            printf("\a"); //todo: shouldn't happen, validate
        }
    }

    return c;
}

char StreamInput::GetNextChar(bool *is_replay)
{
    std::unique_lock<std::mutex> lock(m_shared_data->m_mutex);
    while (m_shared_data->m_paused && m_shared_data->m_steps == 0) {
        m_shared_data->m_step_cv.wait(lock);
    }
    --m_shared_data->m_steps;

    if (!m_typeahead.empty())
    {
        char ch = m_typeahead.front();
        m_typeahead.pop_front();
        if (is_replay)
            *is_replay = true;
        return ch;
    }

    char c = ReadChar();

    if (m_version == 'C')
    {
        //wizard keybindings have changed
        if (c == CTRL('W'))
            c = CTRL('P');
        else if (c == CTRL('P'))
            c = CTRL('O');
        else if (c == CTRL('X'))
            c = CTRL('F');
        else if (c == 'X')
            c = CTRL('F');
        else if (c == CTRL('D'))
            c = CTRL('X');
        else if (c == CTRL('U'))
            c = CTRL('R');
        //running has changed
        else if (c == 'J' || c == 'K' || c == 'L' || c == 'H' || c == 'Y' || c == 'U' || c == 'B' || c == 'N') {
            m_typeahead.push_back(tolower(c));
            c = 'f';
        }
    }


    if (!*m_stream || m_shared_data->m_canceled) {
        OnStreamEnd();
        return 0;
    }

    int time = m_shared_data->m_throttle;
    lock.unlock();
    go_to_sleep(time);

    if (is_replay)
        *is_replay = true;
    return c;
}

std::string StreamInput::ReadString()
{
    int size;
    read(*m_stream, &size);

    char buf[256];
    memset(buf, 0, 256);

    if (*m_stream) {
        if (size > 255)
        {
            assert(false);
            size = 255;
        }
        m_stream->read(buf, size);
    }

    return buf;
}

std::string StreamInput::GetNextString(int size)
{
    std::unique_lock<std::mutex> lock(m_shared_data->m_mutex);
    while (m_shared_data->m_paused && m_shared_data->m_steps == 0) {
        m_shared_data->m_step_cv.wait(lock);
    }
    --m_shared_data->m_steps;

    std::string s = ReadString();

    if (!*m_stream || m_shared_data->m_canceled) {
        OnStreamEnd();
        return "";
    }

    int time = m_shared_data->m_throttle;
    lock.unlock();
    go_to_sleep(time);

    return s;
}

void StreamInput::Serialize(std::ostream& out)
{
}

void StreamInput::OnStreamEnd()
{
    m_shared_data->m_stream_empty = true;
    NotifyReplayEnd();
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
}

void StreamInput::ThreadData::OnEmptyStream()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stream_empty = true;
}

void StreamInput::OnReplayEnd(const std::function<void()>& handler)
{
    m_on_end = handler;
}

void StreamInput::OnFastPlayChanged(const std::function<void(bool)>& handler)
{
    m_on_fast_play = handler;
}

void StreamInput::NotifyReplayEnd()
{
    if (m_on_end)
        m_on_end();
}

void StreamInput::NotifyFastPlayChanged(bool enable)
{
    if (m_on_fast_play)
        m_on_fast_play(enable);
}

//todo:
//serialize enter and esc
//validate serialization
//serialize sizeof
