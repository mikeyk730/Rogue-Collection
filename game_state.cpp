#include <fstream>
#include <algorithm>
#include "rogue.h"
#include "game_state.h"
#include "random.h"
#include "input_interface.h"
#include "stream_input.h"
#include "captured_input.h"
#include "keyboard_input.h"
#include "combo_input.h"
#include "hero.h"
#include "level.h"
#include "console_output.h"
#include "mach_dep.h"

namespace
{
    const int s_serial_version = 2;
}

GameState::GameState(int seed) :
m_seed(seed),
m_input_interface(new CapturedInput(new KeyboardInput())),
m_output_interface(new ConsoleOutput({0,0})),
m_level(new Level),
m_hero(new Hero),
m_scrolls(new ScrollInfo),
m_potions(new PotionInfo),
m_rings(new RingInfo),
m_sticks(new StickInfo)
{
    init_environment();
}

GameState::GameState(Random* random, std::istream& in) :
    m_output_interface(new ConsoleOutput({0,0})),
    m_allow_fast_play(false)
{
    int version = 0;
    read(in, &version);

    if (version == 1 || version == 2) {
        read(in, &m_seed);
        read(in, &m_restore_count);

        int length = 0;
        read(in, &length);
        while (length-- > 0) {
            std::string key, value;
            read_string(in, &key);
            read_string(in, &value);
            m_environment[key] = value;
        }
    }
    else
    {
        // original code didn't write a version, so what we've already read is the seed
        m_seed = version;
        init_environment();
    }    

    ++m_restore_count;
    random->set_seed(m_seed);
    m_input_interface.reset(new CapturedInput(new ComboInput(new StreamInput(in, version), new KeyboardInput())));
    m_level.reset(new Level);
    m_hero.reset(new Hero);
    m_scrolls.reset(new ScrollInfo);
    m_potions.reset(new PotionInfo);
    m_rings.reset(new RingInfo);
    m_sticks.reset(new StickInfo);
}

GameState::~GameState()
{ }

void GameState::init_environment()
{
    m_environment["name"] = "Rodney";
    m_environment["scorefile"] = "rogue.scr";
    m_environment["savefile"] = "rogue.sav";
    m_environment["macro"] = "v";
    m_environment["fruit"] = "Slime Mold"; //move to Hero?
    m_environment["menu"] = "on";
    m_environment["screen"] = "";
    m_environment["levelnames"] = "on";
    m_environment["monstercfg"] = "monsters.opt";
}

void GameState::save_game(const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary | std::ios::out);

    write(file, s_serial_version);
    write(file, m_seed);
    write(file, m_restore_count);

    write(file, m_environment.size());
    for (auto i = m_environment.begin(); i != m_environment.end(); ++i) {
        write_string(file, i->first);
        write_string(file, i->second);
    }

    m_input_interface->Serialize(file);
}

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

OutputInterface & GameState::screen()
{
    return *m_output_interface;
}

Hero& GameState::hero()
{
    return *m_hero;
}

Level & GameState::level()
{
    return *m_level;
}

ScrollInfo& GameState::scrolls()
{
    return *m_scrolls;
}

PotionInfo& GameState::potions()
{
    return *m_potions;
}

RingInfo& GameState::rings()
{
    return *m_rings;
}

StickInfo& GameState::sticks()
{
    return *m_sticks;
}

ItemClass& GameState::item_class(int type)
{
    switch (type)
    {
    case SCROLL:
        return scrolls();
    case POTION:
        return potions();
    case STICK:
        return sticks();
    case RING:
        return rings();
    }
    throw std::runtime_error("Requested bad type of item.");
}

Cheats & GameState::wizard()
{
    return cheats;
}

bool GameState::allow_fast_play() const
{
    return m_allow_fast_play;
}

//todo:
//monster config
//all extern/global variables
//all static variables
