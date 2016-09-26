#include "combo_input.h"

ComboInput::ComboInput(InputInterface * primary, InputInterface * backup) :
    m_active(primary),
    m_backup(backup)
{
    MaybeSwap();
}

bool ComboInput::HasMoreInput()
{
    return m_active->HasMoreInput();
}

char ComboInput::GetNextChar()
{
    char c = m_active->GetNextChar();
    MaybeSwap();
    return c;
}

std::string ComboInput::GetNextString(int size)
{
    std::string s = m_active->GetNextString(size);
    MaybeSwap();
    return s;
}

void ComboInput::Serialize(std::ostream & out)
{
}

void ComboInput::MaybeSwap()
{
    if (!m_active->HasMoreInput() && m_backup)
        m_active = std::move(m_backup);
}
