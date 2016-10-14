#include <fstream>
#include <algorithm>
#include "rogue.h"
#include "game_state.h"
#include "random.h"
#include "input_interface.h"
#include "display_interface.h"
#include "stream_input.h"
#include "captured_input.h"
#include "combo_input.h"
#include "hero.h"
#include "level.h"
#include "output_shim.h"
#include "io.h"
#include "monsters.h"
#include "things.h"

using std::placeholders::_1;

/*
;------------------------------------------------------------------------------
; hit_plus_bugfix(true, false): true
;------------------------------------------------------------------------------
; During a fight, an attacker gets a +4 hit bonus if the defender is sleeping 
; or held.  In the original code, the player was always considered asleep, so 
; monsters always had an increased probability of hitting the player.  I've 
; fixed the bug by default, but added an option to disable the fix.
;
;------------------------------------------------------------------------------
; throws_affect_mimics(true, false): false
;------------------------------------------------------------------------------
; In the original code, a thrown objects would always pass through a disguised 
; mimic.  A zapped bolt would reveal a mimic, but only hit him if the player 
; were blind.  In many cases the mimic would start chasing you while still 
; disguised.
;
; I've simplified this.  If 'throws_affect_mimics' is 'true' you can zap and 
; throw items at a disguised mimic just like any other monster.  If it is 
; set to 'false' then zaps and thrown items will pass through a disguised mimic
;
*/


namespace
{
    const int s_serial_version = 6;
}

GameState::GameState(int seed, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterfaceEx> input) :
    m_seed(seed),
    m_input_interface(new CapturedInput(input)),
    m_curses(new OutputShim(output)),
    m_level(new Level),
    m_hero(new Hero),
    m_log_stream("lastgame.log")
{
    LoadScrolls("scrolls.dat");
    LoadPotions("potions.dat");
    LoadRings("rings.dat");
    LoadSticks("sticks.dat");
    init_environment();
}

GameState::GameState(Random* random, const std::string& filename, bool show_replay, bool start_paused, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterfaceEx> input) :
    m_curses(new OutputShim(output)),
    m_in_replay(true),
    m_show_replay(show_replay),
    m_log_stream("lastgame.log")
{
    std::unique_ptr<std::istream> in(new std::ifstream(filename, std::ios::binary | std::ios::in));
    if (!*in) {
        throw std::runtime_error("Couldn't open " + filename);
    }

    int version = 0;
    read(*in, &version);

    if (version < 3 || version > s_serial_version) {
        throw std::runtime_error("Unsupported save version " + filename);
    }
    if (version < 4) {
        //the game engine has since enabled this bugfix by default
        set_environment("hit_plus_bugfix", "false");
    }
    if (version < 6) {
        //the game engine has since enabled this bugfix by default
        set_environment("ice_monster_miss_bugfix", "false");
    }
    read(*in, &m_seed);
    read(*in, &m_restore_count);

    int env_length = 0;
    read(*in, &env_length);
    while (env_length-- > 0) {
        std::string key, value;
        read_string(*in, &key);
        read_string(*in, &value);
        m_environment[key] = value;
    }
    process_environment();

    if (version >= 5) {
        int mon_length = 0;
        read(*in, &mon_length);
        while (mon_length-- > 0) {
            std::string line;
            read_string(*in, &line);
            load_monster_cfg_entry(line);
        }
    }

    random->set_seed(m_seed);
    ++m_restore_count;

    //Handles replay from save file
    std::unique_ptr<StreamInput> replay_interface(new StreamInput(std::move(in), version, start_paused));
    replay_interface->OnReplayEnd(std::bind(&GameState::set_replay_end, this));
    replay_interface->OnFastPlayChanged(std::bind(&GameState::set_fast_play, this, _1));
    
    //Handles switching to keyboard input when replay is finished
    std::unique_ptr<ComboInput> combo(new ComboInput(std::move(replay_interface), input));

    m_input_interface.reset(new CapturedInput(std::move(combo)));

    m_level.reset(new Level);
    m_hero.reset(new Hero);
    LoadScrolls("scrolls.dat");
    LoadPotions("potions.dat");
    LoadRings("rings.dat");
    LoadSticks("sticks.dat");

    if (!m_show_replay)
    {
        this->screen().stop_rendering();
    }
}

GameState::~GameState()
{ }

bool GameState::in_smart_run_mode() const
{
    return in_run_cmd() && stop_at_door() && !first_move();
}

void GameState::cancel_repeating_cmd()
{
    last_turn.command.count = 0;
}

void GameState::init_environment()
{
    m_environment["name"] = "Rodney";
    m_environment["fruit"] = "Slime Mold";
    m_environment["macro"] = "v";
    m_environment["scorefile"] = "rogue.scr";
    m_environment["savefile"] = "rogue.sav";
    m_environment["powers"] = "jump_levels,reveal_items";

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

    write(file, m_monster_data.size());
    for (auto i = m_monster_data.begin(); i != m_monster_data.end(); ++i) {
        write_string(file, *i);
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

InputInterfaceEx& GameState::input_interface()
{
    return *m_input_interface;
}

OutputShim& GameState::screen()
{
    return *m_curses;
}

Hero& GameState::hero()
{
    return *m_hero;
}

Level & GameState::level()
{
    return *m_level;
}

Cheats& GameState::wizard()
{
    return cheats;
}

bool GameState::in_replay() const
{
    return m_in_replay;
}

void GameState::set_replay_end()
{
    if (!m_show_replay)
    {
        screen().resume_rendering();
    }
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

void GameState::set_monster_data(std::string s)
{
    m_monster_data.push_back(std::move(s));
}

//todo:
//monster config
//all extern/global variables
//all static variables

bool GameState::Options::throws_affect_mimics() const
{
    return game->get_environment("throws_affect_mimics") == "true";
}

bool GameState::Options::act_like_v1_1() const
{
    return game->get_environment("version") == "1.1";
}

bool GameState::Options::show_inventory_menu() const
{
    return game->get_environment("menu") != "false";
}

bool GameState::Options::narrow_screen() const
{
    return game->get_environment("screen_size") == "narrow";
}

bool GameState::Options::monochrome() const
{
    return game->get_environment("screen") == "bw";
}

bool GameState::Options::use_exp_level_names() const
{
    return game->get_environment("use_exp_level_names") != "false";
}

bool GameState::Options::show_torch() const
{
    return game->get_environment("torch") == "true";
}

bool GameState::Options::hit_plus_bugfix() const
{
    return game->get_environment("hit_plus_bugfix") != "false";
}

bool GameState::Options::ice_monster_miss_bugfix() const
{
    return game->get_environment("ice_monster_miss_bugfix") != "false";
}

int GameState::get_level()
{
    return m_level_number;
}

int GameState::next_level()
{
    ++m_level_number;
    if (m_level_number > m_max_level)
        m_max_level = m_level_number;
    return m_level_number;
}

int GameState::prev_level()
{
    return --m_level_number;
}

int GameState::max_level()
{
    return m_max_level;
}

