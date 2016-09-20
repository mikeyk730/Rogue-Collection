#include <sstream>
#include <cassert>
#include <cstdio>
#include "stream_input.h"

StreamInput::StreamInput(std::istream& in, InputInterface* backup) :
m_backup(backup),
m_stream(in)
{ }

char StreamInput::GetNextChar()
{
    char c;
    m_stream.read(&c, 1);

    if (m_stream && c == 0) //shouldn't happen
    {
        printf("\a");
    }

    if (!m_stream){
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
