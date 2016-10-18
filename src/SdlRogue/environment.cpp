#include "environment.h"
#include <iostream>
#include <sstream>
#include <set>
#include <map>

namespace
{
    std::set<std::string> s_unix_bools = { "terse","flush","jump","seefloor","tombstone","passgo","step","askme" };
    std::map<std::string, std::string> s_unix_keys = { {"savefile", "file"}, {"name", "name"}, {"fruit", "fruit"}, {"inven", "inven"} };

    void write_env_str(std::ostringstream& ss, std::string key, const std::string & value)
    {
        auto i = s_unix_keys.find(key);
        if (i != s_unix_keys.end()) {
            ss << i->second << "=" << value << ',';
        }

        if (s_unix_bools.find(key) != s_unix_bools.end()) {
            if (value != "true")
                ss << "no";
            ss << key << ',';
        }
    }
}

Environment::Environment()
{}

void Environment::from_file(std::istream & in)
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

bool Environment::write_to_os()
{
    std::ostringstream ss;
    ss << "ROGUEOPTS=";
    for (auto i = m_environment.begin(); i != m_environment.end(); ++i)
    {
        write_env_str(ss, i->first, i->second);
    }
    return _putenv(ss.str().c_str()) == 0;
}

int Environment::lines() const
{
    return m_lines;
}

int Environment::cols() const
{
    return m_cols;
}
