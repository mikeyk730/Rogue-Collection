#include <algorithm>
#include "captured_input.h"
#include "rogue.h"

CapturedInput::CapturedInput(InputInterface* d)
: m_delegate(d)
{ }

char CapturedInput::GetNextChar()
{
    char c = m_delegate->GetNextChar();
    m_stream.push_back(fastmode ? ON : OFF);
    m_stream.push_back(fast_play_enabled ? ON : OFF);
    m_stream.push_back(stop_at_door ? ON : OFF);
    m_stream.push_back(running ? ON : OFF);
    m_stream.push_back(c);
    return c;
}

std::string CapturedInput::GetNextString(int size)
{
    std::string s = m_delegate->GetNextString(size);
    m_stream.push_back(0);
    m_stream.insert(m_stream.end(), s.begin(), s.end());
    m_stream.push_back(0);
    return s;
}

void CapturedInput::Serialize(std::ostream& out)
{
    out.write((const char*)&m_stream[0], m_stream.size());
}
