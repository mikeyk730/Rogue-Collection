#pragma once
#include <memory>
#include <map>
#include <string>
#include <fstream>
#include "util.h"
#include "item_class.h"
#include "wizard.h"
#include "command.h"

struct Random;
struct InputInterface;
struct Hero;
struct Level;
struct OutputShim;
struct DisplayInterface;

struct GameState
{
    GameState(int seed, std::shared_ptr<DisplayInterface> output, std::shared_ptr<InputInterface> input);
    GameState(Random* random, const std::string& filename, bool show_replay, bool start_paused, std::shared_ptr<DisplayInterface> output, std::shared_ptr<InputInterface> input);
    ~GameState();

    void save_game(const std::string& filename);

    std::string get_environment(const std::string& key) const;
    void set_environment(const std::string& key, const std::string& value);
    void process_environment();

    void log(const std::string& category, const std::string& msg);

    Random& random();
    InputInterface& input_interface();
    OutputShim& screen();
    Level& level();
    Hero& hero();
    
    ScrollInfo& scrolls();
    PotionInfo& potions();
    RingInfo& rings();
    StickInfo& sticks();
    ItemClass* item_class(int type);

    Cheats& wizard();

    bool in_replay() const;
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

    struct Options
    {
        //control options
        bool show_inventory_menu() const;
        bool stop_running_at_doors() const;

        //graphics options
        bool narrow_screen() const;
        bool monochrome() const;
        bool use_exp_level_names() const;
        bool show_torch() const;

        //rule changing options
        bool hit_plus_bugfix() const;
        bool ice_monster_miss_bugfix() const;
        bool use_throw_damage() const;
        bool throws_affect_mimics() const;
        bool act_like_v1_1() const;
    } options;

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
    int maxrow;            //Last Line used for map
    int msg_position = 0;  //Where cursor is on top line
    std::string typeahead;
    std::string macro;

private:
    void init_environment();

    long m_seed = 0; //Random number seed
    int m_restore_count = 0;
    bool m_fast_play_enabled = false; //If 'Fast Play' has been enabled
    bool m_in_replay = false;         //If we are currently replaying a saved game
    bool m_show_replay = true;        //If we render the game during a replay
    int m_level_number = 1;           //Which floor of the dungeon we're on
    int m_max_level = 1;              //Maximum floor reached

    std::map<std::string, std::string> m_environment; //customizable environment strings 

    std::unique_ptr<Random> m_random; //Random number generator
    std::shared_ptr<InputInterface> m_input_interface; //Interface for getting game input
    std::unique_ptr<OutputShim> m_curses;
    std::unique_ptr<Level> m_level;
    std::unique_ptr<Hero> m_hero;
    
    std::unique_ptr<ScrollInfo> m_scrolls;
    std::unique_ptr<PotionInfo> m_potions;
    std::unique_ptr<RingInfo> m_rings;
    std::unique_ptr<StickInfo> m_sticks;

    std::vector<std::pair<std::string, std::string>> m_log;
    std::ofstream m_log_stream;
    std::vector<std::string> m_monster_data;

    Cheats cheats;
 };
