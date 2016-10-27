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
    void Set(const std::string& key, const std::string& value);
    void LoadFromFile(const std::string& name);
    void ApplyArgs(const Args& args);

    int Lines() const;
    void Lines(int n);

    int Columns() const;
    void Columns(int n);

    int WindowScaling() const;
    bool Fullscreen() const;

    bool WriteToOs(bool for_unix);
    void Serialize(std::ostream& file);
    void Deserialize(std::istream& in);

private:
    std::map<std::string, std::string> m_environment;
    int m_lines = 25;
    int m_cols = 80;
};