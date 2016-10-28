#include <QMessageBox>

void ErrorBox(const std::string& title, const std::string& msg)
{
    QMessageBox::critical(nullptr, title.c_str(), msg.c_str());
}

