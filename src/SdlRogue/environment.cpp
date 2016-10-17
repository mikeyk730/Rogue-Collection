#include "environment.h"
#include <iostream>

Environment::Environment(std::istream & in)
{
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        size_t p = line.find_first_of('=');
        if (p == std::string::npos)
            continue;
        std::string key = line.substr(0, p);
        std::string value = line.substr(p+1, std::string::npos);
        set(key, value);
    }
}

bool Environment::get(const std::string & key, std::string* value) const
{
    auto i = m_environment.find(key);
    if (i != m_environment.end()) {
        if (value)
            *value = i->second;
        return true;
    }
    return false;
}

void Environment::set(const std::string & key, const std::string & value)
{
    m_environment[key] = value;
}

void Environment::lines(int n)
{
    m_lines = n;
}

void Environment::cols(int n)
{
    m_cols = n;
}

int Environment::lines() const
{
    return m_lines;
}

int Environment::cols() const
{
    return m_cols;
}
