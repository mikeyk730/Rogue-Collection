#include "combo_input.h"

ComboInput::ComboInput(std::shared_ptr<InputInterfaceEx> primary, std::shared_ptr<InputInterfaceEx> backup) :
    m_active(primary),
    m_backup(backup)
{
    Swap();
}

bool ComboInput::HasMoreInput()
{
    return m_active->HasMoreInput();
}

char ComboInput::GetNextChar(bool *is_replay)
{
    char c = m_active->GetNextChar(is_replay);
    if (Swap() && c == 0) {
        return m_active->GetNextChar(is_replay);
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

bool ComboInput::Swap()
{
    if (!m_active->HasMoreInput() && m_backup) {
        m_active = std::move(m_backup);
        return true;
    }
    return false;
}
