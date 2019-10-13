#pragma once
struct Args;

int StartProcess(int(*start)(Args& args), int argc, char** argv);