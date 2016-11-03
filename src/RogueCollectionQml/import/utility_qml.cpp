#include <QCoreApplication>
#include "utility.h"

void QuitApplication()
{
    QCoreApplication::quit();
}

#ifdef WIN32
#include <Windows.h>
void DisplayMessage(const std::string &type, const std::string &title, const std::string &msg)
{
    int icon = 0;
    if (type == "Error") {
        icon = MB_ICONERROR;
    }
    else if (type == "Warning") {
        icon = MB_ICONWARNING;
    }
    MessageBoxA(0, msg.c_str(), title.c_str(), MB_OK | icon);
}
#endif

