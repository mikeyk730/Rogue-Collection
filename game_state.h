#pragma once
#include <memory>
#include <map>
#include <string>

#include "item_class.h"

struct Random;
struct InputInterface;
struct Hero;

struct GameState
{
    GameState(int seed);
    GameState(Random* random, std::istream& in);
    ~GameState();

    void save_game(const std::string& filename);

    std::string get_environment(const std::string& key) const;
    void set_environment(const std::string& key, const std::string& value);

    Random& random();
    InputInterface& input_interface();
    Hero& hero();
	
	ScrollInfo& scrolls();
	PotionInfo& potions();
	RingInfo& rings();
	StickInfo& sticks();

private:
    void init_environment();

    long m_seed; //Random number seed
    std::map<std::string, std::string> m_environment; //customizable environment strings 

    std::unique_ptr<Random> m_random; //Random number generator
    std::unique_ptr<InputInterface> m_input_interface; //Interface for getting game input
    std::unique_ptr<Hero> m_hero;
	
	ScrollInfo m_scrolls;
	PotionInfo m_potions;
	RingInfo m_rings;
	StickInfo m_sticks;
 };