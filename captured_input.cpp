#include <algorithm>
#include <cassert>
#include "captured_input.h"
#include "rogue.h"
#include "game_state.h"
#include "mach_dep.h"

namespace 
{
    const char s_version = 'B';
}

CapturedInput::CapturedInput(InputInterface* d)
: m_delegate(d)
{ }

bool CapturedInput::HasMoreInput()
{
    return m_delegate->HasMoreInput();
}

char CapturedInput::GetNextChar()
{
    assert(!m_save_pending);
    char c = m_delegate->GetNextChar();
    if (c == 'S') {
        m_save_pending = true;
    }
    else {
        m_stream.push_back(game->modifiers.fast_mode() ? ON : OFF);
        m_stream.push_back(game->modifiers.scroll_lock() ? ON : OFF);
        m_stream.push_back(game->modifiers.stop_at_door() ? ON : OFF);
        m_stream.push_back(game->modifiers.is_running() ? ON : OFF);
        m_stream.push_back(c);
    }
    return c;
}

std::string CapturedInput::GetNextString(int max_size)
{
    std::string s = m_delegate->GetNextString(max_size);
    if (!m_save_pending) {
        int size = s.length();
        m_stream.push_back(size & 0xff);
        m_stream.push_back(size>>8 & 0xff);
        m_stream.push_back(size>>16 & 0xff);
        m_stream.push_back(size>>24 & 0xff);        
        m_stream.insert(m_stream.end(), s.begin(), s.end());
    }
    m_save_pending = false;
    return s;
}

void CapturedInput::Serialize(std::ostream& out)
{
    write<char>(out, s_version);
    out.write((const char*)&m_stream[0], m_stream.size());
}
