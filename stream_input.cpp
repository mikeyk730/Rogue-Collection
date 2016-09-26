#include <sstream>
#include <cassert>
#include <cstdio>
#include <conio.h>
#include "mach_dep.h"
#include "game_state.h"
#include "stream_input.h"
#include "rogue.h"

namespace
{
    DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
    {
        StreamInput* si = (StreamInput*)lpParameter;
        for (;;) {
            while (!_kbhit());
            char c = _getch();
            if (c == 'p')
                si->PausePlayback();
            else if (c == 's')
                si->StepPlayback();
            else if (c == 'r')
                si->ResumePlayback();
            else if (c == 'c') {
                si->CancelPlayback();
                break;
            }
        }
        return 0;
    }    
}


StreamInput::StreamInput(std::istream& in, InputInterface* backup) :
m_backup(backup),
m_stream(in)
{ 
    m_thread = CreateThread(NULL, 0, ThreadProc, this, 0, 0);
}

char StreamInput::GetNextChar()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_paused && m_steps == 0) {
        m_step_cv.wait(lock);
    }
    --m_steps;

    unsigned char f[4];
    m_stream.read((char*)f, 4);
    if (m_stream)
    {
        game->modifiers.m_fast_mode = (f[0] == ON);
        game->modifiers.m_fast_play_enabled = f[1] == ON;
        game->modifiers.m_stop_at_door = f[2] == ON;
        game->modifiers.m_running = f[3] == ON;
    }

   // while (!_kbhit());
    //getkey();

    char c;
    m_stream.read(&c, 1);

    if (m_stream && c == 0) //shouldn't happen
    {
        printf("\a");
    }

    if (!m_stream){
        OnStreamEnd();
        if (m_backup){
            return m_backup->GetNextChar();
        }
        else{
            for (;;);
        }
    }
    return c;
}

std::string StreamInput::GetNextString(int size)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_paused && m_steps == 0) {
        m_step_cv.wait(lock);
    }
    --m_steps;

    std::ostringstream ss;

    char c;
    m_stream.read(&c, 1);

    if (m_stream && c != 0) //shouldn't happen
    {
        printf("\a");
    }

    for (int i = 0; m_stream && i < 255; ++i){
        m_stream.read(&c, 1);
        if (c == 0)
            break;
        ss << c;
    }

    if (ss.str().size() > 150){ //shouldn't happen
        printf("\a");
    }

    if (!m_stream){
        OnStreamEnd();
        
        if (m_backup){
            return m_backup->GetNextString(size);
        }
        else{
            for (;;);
        }
    }

    return ss.str();
}

void StreamInput::Serialize(std::ostream& out) 
{
}

//todo: gets called too late.  need to peek ahead in stream
void StreamInput::OnStreamEnd()
{
    game->m_allow_fast_play = true;
    if (m_thread) {
        TerminateThread(m_thread, 0);
        CloseHandle(m_thread);
        m_thread = 0;
    }
}

void StreamInput::PausePlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = true;
    m_steps = 0;
}

void StreamInput::StepPlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_steps++;
    m_step_cv.notify_all();
}

void StreamInput::ResumePlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = false;
    m_steps = 0;
    m_step_cv.notify_all();
}

void StreamInput::CancelPlayback()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stream.setstate(std::ios::failbit);
    m_paused = false;
    m_steps = 0;
    m_step_cv.notify_all();
}

//todo:
//serialize enter and esc
//validate serialization
//serialize sizeof
