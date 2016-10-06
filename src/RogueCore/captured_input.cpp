#include <algorithm>
#include <cassert>
#include "captured_input.h"
#include "rogue.h"
#include "game_state.h"
#include "io.h"

namespace
{
    const char s_version = 'C';
}

CapturedInput::CapturedInput(std::shared_ptr<InputInterface> d)
    : m_delegate(d)
{ }

bool CapturedInput::HasMoreInput()
{
    return m_delegate->HasMoreInput();
}

char CapturedInput::GetNextChar()
{
    //mdk: we may discard a 'S' keystroke that is not part of a save.
    //For instance, the user can give bad input in a menu.  I'm willing
    //to live with throwing it away, as I don't think it can have any
    //effect.
    m_save_pending = false;
    char c = m_delegate->GetNextChar();
    if (c == 'S') {
        m_save_pending = true;
    }
    else {
        m_stream.push_back(game->fast_play() ? 1 : 0);
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
        m_stream.push_back(size >> 8 & 0xff);
        m_stream.push_back(size >> 16 & 0xff);
        m_stream.push_back(size >> 24 & 0xff);
        m_stream.insert(m_stream.end(), s.begin(), s.end());
    }
    m_save_pending = false;
    return s;
}

bool CapturedInput::IsCapsLockOn()
{
    return m_delegate->IsCapsLockOn();
}

bool CapturedInput::IsNumLockOn()
{
    return m_delegate->IsNumLockOn();
}

bool CapturedInput::IsScrollLockOn()
{
    return m_delegate->IsScrollLockOn();
}

void CapturedInput::Serialize(std::ostream& out)
{
    write<char>(out, s_version);
    out.write((const char*)&m_stream[0], m_stream.size());
}
