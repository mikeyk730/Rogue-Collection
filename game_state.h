#pragma once
#include <memory>
#include <map>
#include <string>
#include <fstream>
#include "rogue.h"
#include "item_class.h"
#include "wizard.h"

struct Random;
struct InputInterface;
struct Hero;
struct Level;
struct OutputInterface;

struct GameState
{
    GameState(int seed);
    GameState(Random* random, std::istream& in);
    ~GameState();

    void save_game(const std::string& filename);

    std::string get_environment(const std::string& key) const;
    void set_environment(const std::string& key, const std::string& value);
    void process_environment();

    void log(const std::string& category, const std::string& msg);

    Random& random();
    InputInterface& input_interface();
    OutputInterface& screen();
    Level& level();
    Hero& hero();
    
    ScrollInfo& scrolls();
    PotionInfo& potions();
    RingInfo& rings();
    StickInfo& sticks();
    ItemClass* item_class(int type);

    Cheats& wizard();

    bool is_replay() const; //todo:kill this
    bool m_allow_fast_play = true;

    CommandModifiers modifiers;

    struct LastTurnState
    {
        //State about the last command, needed to support commands like 'a' (repeat last action)

        byte command;             //Last command executed
        int count;                //Count of last command executed
        bool could_pickup;        //Could the player pickup items (was 'g' not pressed last turn)

        Coord input_direction;     //Direction the player specified for the last throw, zap, etc.
        Item* item_used = nullptr; //Last item used (quaffed, zapped, thrown, dropped, etc.)
        byte item_letter = 0;      //The last letter selected from inventory

    } last_turn;

    struct Options
    {
        bool stop_running_at_doors() const;
        bool throws_affect_mimics() const;
        bool show_inventory_menu() const;
        bool narrow_screen() const;
        bool monochrome() const;
        bool use_exp_level_names() const;
        bool zap_release_bugfix() const;
        bool striking_charge_bugfix() const;
        bool use_throw_damage() const;
        bool aggressive_orcs() const;

    } options;

    bool is_running() const { return m_running; } // todo:move to Hero?
    void stop_running() { m_running = false; }
    bool stop_at_door() const { return m_stop_at_door; }
    bool first_move() const { return m_first_move; }


    // uncategorized state.  //todo: categorize it.
    bool m_running = false;       //True if player is running //todo: i really need to understand this one
    bool m_stop_at_door = false;  //Stop running when we pass a door
    bool m_first_move = false;     //First move after setting stop_at_door

    int bear_trap_turns = 0;     //Number of turns held in place
    int sleep_timer = 0;         //Number of turns asleep
    int no_food = 0;             //Number of levels without food
    int turns_since_heal = 0;    //Number of turns_since_heal turns
    bool can_pickup_this_turn = 0;  //Will the rogue pick up an item if he steps on it
    bool repeat_last_action;    //The last command is repeated
    int was_trapped = 0;   //Was a trap sprung
    bool invalid_position = false;
    char run_character; //Direction player is running
    char last_message[BUFSIZE];     //The last message printed
    int maxrow;            //Last Line used for map
    int msg_position = 0;  //Where cursor is on top line
    int repeat_cmd_count = 0;  //Number of times to repeat command
    Coord oldpos; //Position before last look() call
    struct Room *oldrp; //room of oldpos
    std::string typeahead;
    std::string macro;

private:
    void init_environment();

    long m_seed = 0; //Random number seed
    int m_restore_count = 0;
    std::map<std::string, std::string> m_environment; //customizable environment strings 

    std::unique_ptr<Random> m_random; //Random number generator
    std::unique_ptr<InputInterface> m_input_interface; //Interface for getting game input
    std::unique_ptr<OutputInterface> m_output_interface;
    std::unique_ptr<Level> m_level;
    std::unique_ptr<Hero> m_hero;
    
    std::unique_ptr<ScrollInfo> m_scrolls;
    std::unique_ptr<PotionInfo> m_potions;
    std::unique_ptr<RingInfo> m_rings;
    std::unique_ptr<StickInfo> m_sticks;

    std::vector<std::pair<std::string, std::string>> m_log;
    std::ofstream m_log_stream;

    Cheats cheats;
 };
