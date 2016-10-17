#pragma once
#include <string>
#include <map>
#include <iosfwd>

struct Environment
{
    Environment(std::istream& in);
    bool get(const std::string& key, std::string* value) const;
    void set(const std::string& key, const std::string& value);

    int lines() const;
    void lines(int n);

    int cols() const;
    void cols(int n);

private:
    std::map<std::string, std::string> m_environment;
    int m_lines = 25;
    int m_cols = 80;
};