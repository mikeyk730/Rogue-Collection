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

    void WriteEnvUnix(std::ostringstream& ss, std::string key, const std::string & value)
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

    void WriteEnvPc(std::ostringstream& ss, std::string key, const std::string & value)
    {
        ss << key << "=" << value << ';';
    }
}

Environment::Environment()
{}

void Environment::LoadFromFile(const std::string& name)
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
        Set(key, value);
    }
}

void Environment::Deserialize(std::istream& in)
{
    uint16_t env_length = 0;
    Read(in, &env_length);

    while (env_length-- > 0) {
        std::string key, value;
        ReadShortString(in, &key);
        ReadShortString(in, &value);
        m_environment[key] = value;
    }
}

void Environment::Serialize(std::ostream& file)
{
    uint16_t env_length = static_cast<uint16_t>(m_environment.size());
    Write(file, env_length);

    for (auto i = m_environment.begin(); i != m_environment.end(); ++i) {
        WriteShortString(file, i->first);
        WriteShortString(file, i->second);
    }
}

bool Environment::Get(const std::string & key, std::string* value) const
{
    auto i = m_environment.find(key);
    if (i != m_environment.end()) {
        if (value)
            *value = i->second;
        return true;
    }
    return false;
}

void Environment::Set(const std::string & key, const std::string & value)
{
    m_environment[key] = value;
}

void Environment::Lines(int n)
{
    m_lines = n;
}

void Environment::Columns(int n)
{
    m_cols = n;
}

bool Environment::WriteToOs(bool for_unix)
{
    std::ostringstream ss;
    ss << "ROGUEOPTS=";
    for (auto i = m_environment.begin(); i != m_environment.end(); ++i)
    {
        if (for_unix)
            WriteEnvUnix(ss, i->first, i->second);
        else
            WriteEnvPc(ss, i->first, i->second);
    }
    if (_putenv(ss.str().c_str()) != 0)
        return false;

    std::string seed;
    if (!Get("seed", &seed))
        return false;

    ss.str("");
    ss << "SEED=" << seed;    
    return (_putenv(ss.str().c_str()) == 0);
}

int Environment::Lines() const
{
    return m_lines;
}

int Environment::Columns() const
{
    return m_cols;
}
