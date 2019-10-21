#pragma once
#include <string>

struct DisplayInterface;
struct InputInterface;
struct Args;
struct Environment;

void RunGame(const std::string& lib, DisplayInterface* display, InputInterface* input, Environment* environment, int lines, int columns, const Args& args);