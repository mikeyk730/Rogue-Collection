#include "game_state.h"
#include "random.h"
#include "input_interface.h"
#include "keyboard_input.h"

GameState::GameState(int seed) :
m_seed(seed),
m_random(new Random(seed)),
m_input_interface(new KeyboardInput)
{

    m_environment["name"] = "Rodney";
    m_environment["scorefile"] = "rogue.scr";
    //m_environment["savefile"] = "rogue.sav";
    m_environment["macro"] = "v";
    m_environment["fruit"] = "Slime Mold";
    m_environment["menu"] = "on";
    m_environment["screen"] = "";
    m_environment["levelnames"] = "on";
    m_environment["monstercfg"] = "monsters.opt";
}

GameState::~GameState()
{ }

std::string GameState::get_environment(const std::string& key) const
{
    auto i = m_environment.find(key);
    if (i != m_environment.end()){
        return i->second;
    }
    return "";
}

void GameState::set_environment(const std::string& key, const std::string& value)
{
    m_environment[key] = value;
}

Random& GameState::random()
{
    return *m_random;
}

InputInterface& GameState::input_interface()
{
    return *m_input_interface;
}

//todo:
//monster config
