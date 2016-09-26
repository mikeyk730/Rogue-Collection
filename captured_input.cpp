#include <algorithm>
#include <cassert>
#include "captured_input.h"
#include "rogue.h"
#include "game_state.h"

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

std::string CapturedInput::GetNextString(int size)
{
    std::string s = m_delegate->GetNextString(size);
    if (!m_save_pending) {
        m_stream.push_back(0);
        m_stream.insert(m_stream.end(), s.begin(), s.end());
        m_stream.push_back(0);
    }
    m_save_pending = false;
    return s;
}

void CapturedInput::Serialize(std::ostream& out)
{
    out.write((const char*)&m_stream[0], m_stream.size());
}
