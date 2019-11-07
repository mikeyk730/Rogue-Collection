#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include <map>
#include <climits>
#include <cstring> //todo:mdk move to cross plat
#include "environment.h"
#include "utility.h"
#include "args.h"

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

    void WriteEnvPc(std::ostringstream& ss, const std::string& key, const std::string& value)
    {
        ss << key << "=" << value << ';';
    }

    int SetEnvVariable(const char* envstr) //todo:mdk move to cross platform file
    {
#ifdef __linux__
        char* mem = (char*)malloc(strlen(envstr) + 1);
        strcpy(mem, envstr);
        return putenv(mem);
#elif _WIN32
        return _putenv(envstr);
#endif
    }
}

Environment::Environment()
{
    SetDefaults();
}

Environment::Environment(const Args& args)
{
    SetDefaults();
    LoadFromFile(args.optfile);
    ApplyArgs(args);
}

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

void Environment::ApplyArgs(const Args& args)
{
    if (args.small_screen)
        Set("small_screen", "true");
    if (!args.gfx.empty())
        Set("gfx", args.gfx);
    if (!args.savefile.empty())
        Set("game", args.savefile);
    if (!args.fontfile.empty())
        Set("font", args.fontfile);
    if (args.start_paused)
        Set("replay_paused", "true");
    if (!args.pause_at.empty())
        Set("replay_pause_at", args.pause_at);
    if (!args.genes.empty())
        Set("genes", args.genes);

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

void Environment::SetRogomaticValues()
{
    Set("prompt_for_name", "false");
    Set("prompt_for_help", "true");
    Set("unix_output", "true");
    Set("name", "rogomatic");
    Set("fruit", "apricot");
    Set("terse", "true");
    Set("jump", "true");
    Set("seefloor", "true");
    Set("flush", "false");
    Set("askme", "false");
    Set("passgo", "false");
    Set("step", "true");
    Set("inven", "slow");
    Set("menu", "false");
    Set("showac", "");
    Set("show_animation", "false");
}

void Environment::SetDefaults()
{
    Set("passgo", "true");
    Set("askme", "true");
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

bool Environment::IsEqual(const std::string & key, const std::string & target_value) const
{
    std::string actual_value;
    return (Get(key, &actual_value) && actual_value == target_value);
}

void Environment::Set(const std::string& key, const std::string& value)
{
    if (value.empty())
        Clear(key);
    else
        m_environment[key] = value;
}

void Environment::Clear(const std::string& key)
{
    auto i = m_environment.find(key);
    if (i != m_environment.end()) {
        m_environment.erase(i);
    }
}

int Environment::WindowScaling() const
{
    int scale = INT_MAX;
    std::string value;
    if (Get("window_scaling", &value))
    {
        scale = atoi(value.c_str());
    }
    return scale;
}

bool Environment::Fullscreen() const
{
    std::string value;
    return Get("fullscreen", &value) && value == "true";
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
    if (SetEnvVariable(ss.str().c_str()) != 0)
        return false;

    std::string value;
    if (Get("genes", &value))
    {
        SetEnvVariable(("GENES=" + value).c_str());
    }
    if (IsEqual("ltm", "false"))
    {
        SetEnvVariable("NOLTM=true");
    }
    if (Get("rogomatic_delay", &value))
    {
        SetEnvVariable(("ROGOMATIC_DELAY=" + value).c_str());
    }
    if (Get("rogomatic_level_delay", &value))
    {
        SetEnvVariable(("ROGOMATIC_LEVEL_DELAY=" + value).c_str());
    }
    if (IsEqual("rogomatic_debug_protocol", "true"))
    {
        SetEnvVariable("ROGOMATIC_DEBUG_PROTOCOL=true");
    }

    std::string seed;
    if (!Get("seed", &seed))
        return false;

    ss.str("");
    ss << "SEED=" << seed;
    return (SetEnvVariable(ss.str().c_str()) == 0);
}
