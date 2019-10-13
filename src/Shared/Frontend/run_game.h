#pragma once
#include <string>

struct DisplayInterface;
struct InputInterface;
struct Args;

void RunGame(const std::string& lib, DisplayInterface* display, InputInterface* input, int lines, int columns, const Args& args);