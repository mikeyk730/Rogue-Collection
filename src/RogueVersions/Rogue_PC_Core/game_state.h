#pragma once
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <coord.h>
#include "wizard.h"
#include "command.h"

struct Random;
struct InputInterfaceEx;
struct Hero;
struct Level;
struct OutputShim;
struct OutputInterface;
struct Item;

struct Options
{
    bool start_replay_paused() const;
    bool hide_replay() const;

    //control options
    bool show_inventory_menu() const;
    bool dir_key_clears_more() const;
    bool prompt_for_name() const;

    //graphics options
    bool narrow_screen() const;
    bool monochrome() const;
    bool use_exp_level_names() const;
    bool show_armor_class() const;

    //rule changing options
    bool hit_plus_bugfix() const;
    bool throws_affect_mimics() const;
    bool act_like_v1_1() const;

    bool disable_scroll_lock() const;
    bool disable_save() const;

public:
    void init_environment();
    void from_file(std::istream & optfile, char delimiter='\n');
    void for_each(std::function<void(std::pair<std::string, std::string>)> f) const;

    std::string get_environment(const std::string& key) const;
    void set_environment(const std::string& key, const std::string& value);
    
    //save environment to replay file
    void Options::serialize(std::ostream& savefile);
    //load environment to replay file
    void deserialize(std::istream& savefile);

private:
    std::map<std::string, std::string> m_environment; //customizable environment strings 
};

struct GameState
{
    GameState(int seed, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterfaceEx> input);
    GameState(Random* random, const std::string& filename, bool show_replay, bool start_paused, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterfaceEx> input);
    ~GameState();

    void CreateHero(const std::string& name);

    void save_game(const std::string& filename);

    void set_logfile(const std::string& filename);
    void log(const std::string& category, const std::string& msg);

    Random& random();
    InputInterfaceEx& input_interface();
    OutputShim& screen();
    Level& level();
    Hero& hero();
    
    Cheats& wizard();

    bool in_replay() const;
    void set_in_replay();
    void set_replay_end();

    bool fast_play() const;
    void set_fast_play(bool enable);

    void load_monster_cfg_entry(const std::string& line);
    void set_monster_data(std::string s);

    int get_level();
    int next_level();
    int prev_level();
    int max_level();

    struct LastTurnState
    {
        //State about the last command, needed to support commands like 'a' (repeat last action)
        Command command;          //Last command executed

        Coord input_direction;     //Direction the player specified for the last throw, zap, etc.
        Item* item_used = nullptr; //Last item used (quaffed, zapped, thrown, dropped, etc.)
        byte item_letter = 0;      //The last letter selected from inventory

    } last_turn;

    Options options;
    void process_environment();

    bool in_smart_run_mode() const;
    bool in_run_cmd() const { return m_running; }
    void stop_run_cmd() { m_running = false; }
    bool stop_at_door() const { return m_stop_at_door; }
    bool first_move() const { return m_first_move; }

    void cancel_repeating_cmd();

    // uncategorized state.  //todo: categorize it.
    bool m_running = false;       //True if player is running //todo: i really need to understand this one
    bool m_stop_at_door = false;  //Stop running when we pass a door
    bool m_first_move = false;     //First move after setting stop_at_door

    int no_food = 0;             //Number of levels without food
    int turns_since_heal = 0;    //Number of turns_since_heal turns
    bool repeat_last_action;     //The last command is repeated
    bool invalid_position = false;
    char run_character; //Direction player is running
    char last_message[128];     //The last message printed
    int msg_position = 0;  //Where cursor is on top line
    std::string typeahead;
    std::string macro;

private:
    long m_seed = 0; //Random number seed
    int m_restore_count = 0;
    bool m_fast_play_enabled = false; //If 'Fast Play' has been enabled
    bool m_in_replay = false;         //If we are currently replaying a saved game
    bool m_show_replay = true;        //If we render the game during a replay
    int m_level_number = 1;           //Which floor of the dungeon we're on
    int m_max_level = 1;              //Maximum floor reached

    std::unique_ptr<Random> m_random; //Random number generator
    std::shared_ptr<InputInterfaceEx> m_input_interface; //Interface for getting game input
    std::unique_ptr<OutputShim> m_curses;
    std::unique_ptr<Level> m_level;
    std::unique_ptr<Hero> m_hero;
    
    std::unique_ptr<std::ofstream> m_log_stream;
    std::vector<std::string> m_monster_data;

    Cheats cheats;
 };
