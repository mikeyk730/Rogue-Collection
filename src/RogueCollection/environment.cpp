#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include <map>
#include "environment.h"
#include "utility.h"

namespace
{
    std::set<std::string> s_unix_bools = { "terse","flush","jump","seefloor","tombstone","passgo","step","askme","hplusfix","showac" };
    std::map<std::string, std::string> s_unix_keys = { {"name", "name"}, {"fruit", "fruit"}, {"inven", "inven"} };

    void write_env_unix(std::ostringstream& ss, std::string key, const std::string & value)
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

    void write_env_pc(std::ostringstream& ss, std::string key, const std::string & value)
    {
        ss << key << "=" << value << ';';
    }
}

Environment::Environment()
{}

void Environment::from_file(const std::string& name)
{
    std::ifstream in(name);

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

void Environment::deserialize(std::istream& in)
{
    uint16_t env_length = 0;
    read(in, &env_length);

    while (env_length-- > 0) {
        std::string key, value;
        read_short_string(in, &key);
        read_short_string(in, &value);
        m_environment[key] = value;
    }
}

void Environment::serialize(std::ostream& file)
{
    uint16_t env_length = static_cast<uint16_t>(m_environment.size());
    write(file, env_length);

    for (auto i = m_environment.begin(); i != m_environment.end(); ++i) {
        write_short_string(file, i->first);
        write_short_string(file, i->second);
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

void Environment::clear()
{
    m_environment.clear();
}

void Environment::lines(int n)
{
    m_lines = n;
}

void Environment::cols(int n)
{
    m_cols = n;
}

bool Environment::write_to_os(bool for_unix)
{
    std::ostringstream ss;
    ss << "ROGUEOPTS=";
    for (auto i = m_environment.begin(); i != m_environment.end(); ++i)
    {
        if (for_unix)
            write_env_unix(ss, i->first, i->second);
        else
            write_env_pc(ss, i->first, i->second);
    }
    if (_putenv(ss.str().c_str()) != 0)
        return false;

    std::string seed;
    if (!get("seed", &seed))
        return false;

    ss.str("");
    ss << "SEED=" << seed;    
    return (_putenv(ss.str().c_str()) == 0);
}

int Environment::lines() const
{
    return m_lines;
}

int Environment::cols() const
{
    return m_cols;
}
