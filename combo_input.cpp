#include "combo_input.h"

ComboInput::ComboInput(InputInterface * primary, InputInterface * backup) :
    m_active(primary),
    m_backup(backup)
{
    Swap();
}

bool ComboInput::HasMoreInput()
{
    return m_active->HasMoreInput();
}

char ComboInput::GetNextChar()
{
    char c = m_active->GetNextChar();
    if (Swap() && c == 0) {
        return m_active->GetNextChar();
    }
    return c;
}

std::string ComboInput::GetNextString(int size)
{
    std::string s = m_active->GetNextString(size);
    if (Swap() && s.empty()) {
        return m_active->GetNextString(size);
    }
    return s;
}

void ComboInput::Serialize(std::ostream & out)
{
}

void ComboInput::OnReplayEnd(const std::function<void()>& handler)
{
    m_active->OnReplayEnd(handler);
    m_backup->OnReplayEnd(handler);
}

void ComboInput::OnFastPlayChanged(const std::function<void(bool)>& handler)
{
    m_active->OnFastPlayChanged(handler);
    m_backup->OnFastPlayChanged(handler);
}

bool ComboInput::Swap()
{
    if (!m_active->HasMoreInput() && m_backup) {
        m_active = std::move(m_backup);
        return true;
    }
    return false;
}
