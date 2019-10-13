#pragma once
#include <string>

struct DisplayInterface;
struct InputInterface;
struct Environment;
struct Args;

void RunGame(const std::string& lib, DisplayInterface* display, InputInterface* input, Environment* environment, const Args& args);