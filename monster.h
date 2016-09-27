#pragma once

#include "agent.h"

struct Monster : public Agent
{
    virtual std::string get_name();
};