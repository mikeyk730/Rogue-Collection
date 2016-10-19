#pragma once
#include <string>
#include <map>
#include <iosfwd>

struct Environment
{
    Environment();

    bool get(const std::string& key, std::string* value) const;
    void set(const std::string& key, const std::string& value);
    void clear();
    void from_file(const std::string& name);

    int lines() const;
    void lines(int n);

    int cols() const;
    void cols(int n);

    bool write_to_os(bool for_unix);
    void serialize(std::ostream& file);
    void deserialize(std::istream& in);

private:
    std::map<std::string, std::string> m_environment;
    int m_lines = 25;
    int m_cols = 80;
};