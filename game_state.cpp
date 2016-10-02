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
#include "io.h"

using std::placeholders::_1;

/*
-------------------------------------------------------------------------------
throws_affect_mimics(true, false): false
-------------------------------------------------------------------------------
In the original code, a thrown objects would always pass through a disguised 
mimic.  A zapped bolt would reveal a mimic, but only hit him if the player were 
blind.  In many cases the mimic would start chasing you while still disguised.

I've simplified this.  If 'throws_affect_mimics' is 'true' you can zap and throw
items at a disguised mimic just like any other monster.  If 'throws_affect_mimics'
is 'false' then zaps and throws will pass through a disguised mimic

*/


namespace
{
    const int s_serial_version = 3;
}

GameState::GameState(int seed) :
    m_seed(seed),
    m_input_interface(new CapturedInput(new KeyboardInput())),
    m_output_interface(new ConsoleOutput({ 0,0 })),
    m_level(new Level),
    m_hero(new Hero),
    m_scrolls(new ScrollInfo),
    m_potions(new PotionInfo),
    m_rings(new RingInfo),
    m_sticks(new StickInfo),
    m_log_stream("log.txt")
{
    init_environment();
}

GameState::GameState(Random* random, const std::string& filename) :
    m_output_interface(new ConsoleOutput({ 0, 0 })),
    m_in_replay(true),
    m_log_stream("log.txt")
{
    std::unique_ptr<std::istream> in(new std::ifstream(filename, std::ios::binary | std::ios::in));
    if (!*in) {
        throw std::runtime_error("Couldn't open " + filename);
    }

    int version = 0;
    read(*in, &version);

    if (version < 3) {
        throw std::runtime_error("Unsupported save version " + filename);
    }

    read(*in, &m_seed);
    read(*in, &m_restore_count);

    int length = 0;
    read(*in, &length);
    while (length-- > 0) {
        std::string key, value;
        read_string(*in, &key);
        read_string(*in, &value);
        m_environment[key] = value;
    }
    process_environment();

    random->set_seed(m_seed);
    ++m_restore_count;

    m_input_interface.reset(new CapturedInput(new ComboInput(new StreamInput(std::move(in), version), new KeyboardInput())));
    m_input_interface->OnReplayEnd(std::bind(&GameState::set_replay_end, this));
    m_input_interface->OnFastPlayChanged(std::bind(&GameState::set_fast_play, this, _1));

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
    m_environment["fruit"] = "Slime Mold";
    m_environment["macro"] = "v";
    m_environment["scorefile"] = "rogue.scr";
    m_environment["savefile"] = "rogue.sav";
    m_environment["monsterfile"] = "rogue.mon";

    process_environment();
}

void GameState::process_environment()
{
    macro = get_environment("macro");
    wizard().add_powers(get_environment("powers"));
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
    if (i != m_environment.end()) {
        return i->second;
    }
    return "";
}

void GameState::set_environment(const std::string& key, const std::string& value)
{
    m_environment[key] = value;
}

void GameState::log(const std::string & category, const std::string & msg)
{
    m_log.push_back(std::make_pair(category, msg));
    m_log_stream << category << ": " << msg << std::endl;
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

ItemClass* GameState::item_class(int type)
{
    switch (type)
    {
    case SCROLL:
        return &scrolls();
    case POTION:
        return &potions();
    case STICK:
        return &sticks();
    case RING:
        return &rings();
    }
    return 0;
}

Cheats & GameState::wizard()
{
    return cheats;
}

bool GameState::in_replay() const
{
    return m_in_replay;
}

void GameState::set_replay_end()
{
    reset_msg_position();
    msg("Replay ended");
    m_in_replay = false;
}

bool GameState::fast_play() const
{
    return m_fast_play_enabled; 
}

void GameState::set_fast_play(bool enable)
{
    m_fast_play_enabled = enable;
}

//todo:
//monster config
//all extern/global variables
//all static variables

bool GameState::Options::stop_running_at_doors() const
{
    return game->get_environment("stop_running_at_doors") == "true";
}

bool GameState::Options::throws_affect_mimics() const
{
    return game->get_environment("throws_affect_mimics") == "true";
}

bool GameState::Options::show_inventory_menu() const
{
    return game->get_environment("menu") != "false";
}

bool GameState::Options::narrow_screen() const
{
    return game->get_environment("narrow_screen") == "true";
}

bool GameState::Options::monochrome() const
{
    return game->get_environment("monochrome") == "true";
}

bool GameState::Options::use_exp_level_names() const
{
    return game->get_environment("use_exp_level_names") != "false";
}

bool GameState::Options::zap_release_bugfix() const
{
    return game->get_environment("zap_release_bugfix") != "false";
}

bool GameState::Options::striking_charge_bugfix() const
{
    return game->get_environment("striking_charge_bugfix") != "false";
}

bool GameState::Options::use_throw_damage() const
{
    return game->get_environment("use_throw_damage") != "false";
}

bool GameState::Options::aggressive_orcs() const
{
    return game->get_environment("orc_type") == "aggressive";
}
