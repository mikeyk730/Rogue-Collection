#include "combo_input.h"

ComboInput::ComboInput(std::shared_ptr<InputInterface> primary, std::shared_ptr<InputInterface> backup) :
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

bool ComboInput::IsCapsLockOn()
{
    return m_active->IsCapsLockOn();
}

bool ComboInput::IsNumLockOn()
{
    return m_active->IsNumLockOn();
}

bool ComboInput::IsScrollLockOn()
{
    return m_active->IsScrollLockOn();
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
