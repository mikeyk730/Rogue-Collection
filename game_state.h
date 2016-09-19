#pragma once
#include <memory>
#include <map>
#include <string>

struct Random;
struct InputInterface;
struct Hero;

struct GameState
{
    GameState(int seed);
    ~GameState();

    void save_game(const std::string& filename);

    std::string get_environment(const std::string& key) const;
    void set_environment(const std::string& key, const std::string& value);

    Random& random();
    InputInterface& input_interface();
    Hero& hero();

private:
    long m_seed; //Random number seed
    std::map<std::string, std::string> m_environment; //customizable environment strings 

    std::unique_ptr<Random> m_random; //Random number generator
    std::unique_ptr<InputInterface> m_input_interface; //Interface for getting game input
    std::unique_ptr<Hero> m_hero;
 };