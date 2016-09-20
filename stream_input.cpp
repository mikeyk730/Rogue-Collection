#include <sstream>
#include <cassert>
#include <cstdio>
#include "stream_input.h"
#include "rogue.h"

StreamInput::StreamInput(std::istream& in, InputInterface* backup) :
m_backup(backup),
m_stream(in)
{ }

#include <conio.h>
#include "mach_dep.h"
#include "game_state.h"

char StreamInput::GetNextChar()
{
    unsigned char f[4];
    m_stream.read((char*)f, 4);
    if (m_stream)
    {
        fastmode = (f[0] == ON);
        fast_play_enabled = f[1] == ON;
        stop_at_door = f[2] == ON;
        running = f[3] == ON;
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
        game->m_allow_fast_play = true;
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
        game->m_allow_fast_play = true;
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
{}

//todo:
//serialize enter and esc
//validate serialization
//serialize sizeof
