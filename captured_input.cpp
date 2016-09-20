#include <algorithm>
#include "captured_input.h"

CapturedInput::CapturedInput(InputInterface* d)
: m_delegate(d)
{ }

char CapturedInput::GetNextChar()
{
    char c = m_delegate->GetNextChar();
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
