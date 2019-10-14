#include <QCoreApplication>
#include "utility_qml.h"

void QuitApplication()
{
    QCoreApplication::quit();
}

std::string GetResourcePath(const std::string& filename)
{
    std::string dir = QCoreApplication::applicationDirPath().toStdString();
    dir += "/res/";
    dir += filename;
    return dir;
}

#ifdef WIN32
#include <Windows.h>
void DisplayMessage(const std::string& type, const std::string& title, const std::string& msg)
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
#else
#include <iostream>
void DisplayMessage(const std::string&, const std::string&, const std::string& msg)
{
    std::cout << msg << std::endl; //todo:mdk
}
#endif

