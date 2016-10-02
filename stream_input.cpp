#include <sstream>
#include <cassert>
#include <cstdio>
#include <conio.h>
#include <thread>
#include "mach_dep.h"
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


StreamInput::StreamInput(std::unique_ptr<std::istream> in, int version) :
    m_stream(std::move(in)),
    m_shared_data(new ThreadData)
{
    if (version <= 1) {
        m_version = 'A';
    }
    else {
        read(*m_stream, &m_version);
    }

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

bool is_direction(byte ch)
{
    switch (ch) {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
        return true;
    default:
        return false;
    }
}

char StreamInput::ReadCharA()
{
    unsigned char f[5];
    m_stream->read((char*)f, 5);

    byte fast_mode(f[0]);
    byte fast_play(f[1]);
    char c(f[4]);

    if (*m_stream)
    {
        NotifyFastPlayChanged(fast_play == ON);
        if (fast_mode == ON)
        {
            if (is_direction(c)) {
                c = toupper(c);
            }
        }

        if (c == 0) //shouldn't happen
        {
            printf("\a");
        }
    }

    return c;
}

char StreamInput::ReadCharC()
{
    unsigned char info[2];
    m_stream->read((char*)info, 2);

    byte fast_play(info[0]);
    char c(info[1]);

    if (*m_stream)
    {
        NotifyFastPlayChanged(fast_play ? true : false);
        if (c == 0)
        {
            printf("\a"); //todo: shouldn't happen, validate
        }
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

    if (!*m_stream || m_shared_data->m_canceled) {
        OnStreamEnd();
        return 0;
    }

    char c;
    if (m_version >= 'A' && m_version <= 'B')
        c = ReadCharA();
    else
        c = ReadCharC();

    return c;
}

std::string StreamInput::ReadStringA()
{
    std::ostringstream ss;

    char c = 0;
    m_stream->read(&c, 1);

    if (*m_stream && c != 0) //shouldn't happen
    {
        printf("\a");
    }

    for (int i = 0; *m_stream && i < 255; ++i) {
        m_stream->read(&c, 1);
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
    read(*m_stream, &size);

    char buf[256];
    memset(buf, 0, 256);
    if (size > 255)
    {
        assert(false);
        size = 255;
    }
    m_stream->read(buf, size);

    return buf;
}

std::string StreamInput::GetNextString(int size)
{
    std::unique_lock<std::mutex> lock(m_shared_data->m_mutex);
    while (m_shared_data->m_paused && m_shared_data->m_steps == 0) {
        m_shared_data->m_step_cv.wait(lock);
    }
    --m_shared_data->m_steps;

    if (!*m_stream || m_shared_data->m_canceled) {
        OnStreamEnd();
        return "";
    }

    std::string s;
    if (m_version == 'A')
        s = ReadStringA();
    else
        s = ReadStringB();


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
