#pragma once
#include <string>

std::string GetResourcePath(const std::string& filename);
void DisplayMessage(const std::string& type, const std::string& title, const std::string& msg);
void QuitApplication();
