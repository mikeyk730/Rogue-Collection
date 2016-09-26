#pragma once
#include <memory>
#include <map>
#include <string>
#include "rogue.h"
#include "item_class.h"

struct Random;
struct InputInterface;
struct Hero;
struct Level;

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
    Level& level();
    Hero& hero();
    
    ScrollInfo& scrolls();
    PotionInfo& potions();
    RingInfo& rings();
    StickInfo& sticks();
    ItemClass& item_class(int type);

    bool allow_fast_play() const; //todo:kill this
    bool m_allow_fast_play = true;

    CommandModifiers modifiers;

private:
    void init_environment();

    long m_seed = 0; //Random number seed
    int m_restore_count = 0;
    std::map<std::string, std::string> m_environment; //customizable environment strings 

    std::unique_ptr<Random> m_random; //Random number generator
    std::unique_ptr<InputInterface> m_input_interface; //Interface for getting game input
    std::unique_ptr<Level> m_level;
    std::unique_ptr<Hero> m_hero;
    
    std::unique_ptr<ScrollInfo> m_scrolls;
    std::unique_ptr<PotionInfo> m_potions;
    std::unique_ptr<RingInfo> m_rings;
    std::unique_ptr<StickInfo> m_sticks;
 };