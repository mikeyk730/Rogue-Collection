#include <fstream>
#include <algorithm>
#include "rogue.h"
#include "game_state.h"
#include "random.h"
#include "input_interface.h"
#include "../Shared/display_interface.h"
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

#ifdef WIN32
#include <Windows.h>
#endif

namespace
{
    const int s_serial_version = 7;

    std::string GetPath(const std::string& subdir) {
#ifdef WIN32
        char buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        std::string s(buffer);
        size_t n = s.find_last_of("\\/");
        return s.substr(0, n+1) + subdir + "\\";
#else
        return "";
#endif
    }
}

GameState::GameState(int seed, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterfaceEx> input) :
    m_seed(seed),
    m_input_interface(new CapturedInput(input)),
    m_curses(new OutputShim(output)),
    m_level(new Level),
    m_hero(new Hero)
{
    std::string path = GetPath("data");
    LoadScrolls(path + "scrolls.dat");
    LoadPotions(path + "potions.dat");
    LoadRings(path + "rings.dat");
    LoadSticks(path + "sticks.dat");

    options.init_environment();
    process_environment();
}

void Options::deserialize(std::istream& in)
{
    int env_length = 0;
    read(in, &env_length);
    while (env_length-- > 0) {
        std::string key, value;
        read_string(in, &key);
        read_string(in, &value);
        m_environment[key] = value;
    }
}


GameState::GameState(Random* random, const std::string& filename, bool show_replay, bool start_paused, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterfaceEx> input) :
    m_curses(new OutputShim(output)),
    m_in_replay(true),
    m_show_replay(show_replay)
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
    if (version == 4 || version == 5) {
        //the game engine has since disabled this bugfix by default
        options.set_environment("hit_plus_bugfix", "true");
    }
    if (version < 6) {
        //the game engine has since enabled this bugfix by default
        options.set_environment("ice_monster_miss_bugfix", "false");
    }
    read(*in, &m_seed);
    read(*in, &m_restore_count);

    options.deserialize(*in);
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
    
    std::string path = GetPath("data");
    LoadScrolls(path + "scrolls.dat");
    LoadPotions(path + "potions.dat");
    LoadRings(path + "rings.dat");
    LoadSticks(path + "sticks.dat");

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

void Options::init_environment()
{
    m_environment["name"] = "Rodney";
    m_environment["fruit"] = "Slime Mold";
    m_environment["macro"] = "v";
    m_environment["scorefile"] = "roguepc.scr";
    m_environment["savefile"] = "rogue.sav";
    m_environment["powers"] = "jump_levels,reveal_items";
    m_environment["throws_affect_mimics"] = "false";
    m_environment["hit_plus_bugfix"] = "false";
    m_environment["ice_monster_miss_bugfix"] = "true";
}

void Options::from_file(const std::string & optfile)
{
    std::ifstream in(optfile);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        size_t p = line.find_first_of('=');
        if (p == std::string::npos)
            continue;
        std::string key = line.substr(0, p);
        std::string value = line.substr(p + 1, std::string::npos);
        set_environment(key, value);
    }
}

void GameState::process_environment()
{
    macro = options.get_environment("macro");
    wizard().add_powers(options.get_environment("powers"));
}

void Options::serialize(std::ostream& file)
{
    write(file, m_environment.size());
    for (auto i = m_environment.begin(); i != m_environment.end(); ++i) {
        write_string(file, i->first);
        write_string(file, i->second);
    }
}

void GameState::save_game(const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary | std::ios::out);

    write(file, s_serial_version);
    write(file, m_seed);
    write(file, m_restore_count);

    options.serialize(file);

    write(file, m_monster_data.size());
    for (auto i = m_monster_data.begin(); i != m_monster_data.end(); ++i) {
        write_string(file, *i);
    }

    m_input_interface->Serialize(file);
}

std::string Options::get_environment(const std::string& key) const
{
    auto i = m_environment.find(key);
    if (i != m_environment.end()) {
        return i->second;
    }
    return "";
}

void Options::set_environment(const std::string& key, const std::string& value)
{
    m_environment[key] = value;
}

void GameState::set_logfile(const std::string & filename)
{
    if (!filename.empty())
        m_log_stream.reset(new std::ofstream(filename));
}

void GameState::log(const std::string & category, const std::string & msg)
{
    if (m_log_stream)
        *m_log_stream << category << ": " << msg << std::endl;
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
    if (!options.disable_scroll_lock())
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

bool Options::throws_affect_mimics() const
{
    return get_environment("throws_affect_mimics") == "true";
}

bool Options::act_like_v1_1() const
{
    return get_environment("version") == "1.1";
}

bool Options::disable_scroll_lock() const
{
    return true;
}

bool Options::disable_save() const
{
    return true;
}

bool Options::show_inventory_menu() const
{
    return get_environment("menu") != "false";
}

bool Options::dir_key_clears_more() const
{
    return get_environment("dir_key_clears_more") == "true";
}

bool Options::prompt_for_name() const
{
    return get_environment("prompt_for_name") != "false";
}

bool Options::start_replay_paused() const
{
    return get_environment("pause_replay") == "true";
}

bool Options::hide_replay() const
{
    return get_environment("hide_replay") == "true";
}

bool Options::narrow_screen() const
{
    return get_environment("small_screen") == "true";
}

bool Options::monochrome() const
{
    return get_environment("screen") == "bw";
}

bool Options::use_exp_level_names() const
{
    return get_environment("use_exp_level_names") != "false";
}

bool Options::hit_plus_bugfix() const
{
    return get_environment("hit_plus_bugfix") == "true";
}

bool Options::ice_monster_miss_bugfix() const
{
    return get_environment("ice_monster_miss_bugfix") != "false";
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

