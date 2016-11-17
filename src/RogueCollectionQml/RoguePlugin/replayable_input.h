#include <mutex>
#include <deque>
#include <vector>
#include <input_interface.h>
#include "game_config.h"

struct Environment;

struct ReplayableInput : public InputInterface
{
public:
    ReplayableInput(Environment* current_env, Environment* game_env, const GameConfig& options);

    //input interface
    virtual char GetChar(bool block, bool for_string, bool *is_replay) override;
    virtual void Flush() override;

    void SaveGame(std::ostream& file);
    void RestoreGame(std::istream& file);

    bool GetRenderText(std::string* text);
    bool InReplay() const;

protected:
    void PauseReplay();
    void ResumeReplay();
    void CancelReplay();
    void DecreaseReplaySpeed();
    void IncreaseReplaySpeed();
    void SetMaxReplaySpeed();
    void SetReplaySpeed(int n);

    void QueueInput(const std::string& input);
    void QueueInput(char input);
    GameConfig Options() const;

private:
    mutable std::mutex m_input_mutex;
    std::condition_variable m_input_cv;
    std::deque<unsigned char> m_buffer;

    GameConfig m_options;
    int m_replay_steps_remaining = 0;
    int m_steps_to_take = 0;
    int m_replay_sleep = 0;
    int m_pause_at = 0;
    bool m_paused = false;

    Environment* m_current_env = 0;
    Environment* m_game_env = 0;

    std::vector<unsigned char> m_keylog;
};
