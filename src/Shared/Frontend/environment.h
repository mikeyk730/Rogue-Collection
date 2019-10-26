#pragma once
#include <string>
#include <map>
#include <iosfwd>

struct Args;

struct Environment
{
    Environment();
    Environment(const Args& args);

    bool Get(const std::string& key, std::string* value) const;
    bool IsEqual(const std::string& key, const std::string& value) const;
    void Set(const std::string& key, const std::string& value);
    void Clear(const std::string& key);
    void LoadFromFile(const std::string& name);
    void ApplyArgs(const Args& args);

    int WindowScaling() const;
    bool Fullscreen() const;

    bool WriteToOs(bool for_unix);
    void Serialize(std::ostream& file);
    void Deserialize(std::istream& in);

    void SetRogomaticValues();

private:
    void SetDefaults();

    std::map<std::string, std::string> m_environment;
};
