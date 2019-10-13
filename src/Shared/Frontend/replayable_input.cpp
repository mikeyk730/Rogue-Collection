#include <sstream>
#include <iterator>
#include "replayable_input.h"
#include "environment.h"
#include "utility.h"

#define ESCAPE 0x1b

ReplayableInput::ReplayableInput(Environment* current_env, Environment* game_env, const GameConfig& options) :
    m_options(options),
    m_current_env(current_env),
    m_game_env(game_env)
{
}

char ReplayableInput::GetChar(bool block, bool for_string, bool *is_replay)
{
    char c = 0;
    int sleep = 0;

    //locked region
    {
        std::unique_lock<std::mutex> lock(m_input_mutex);
        while (m_replay_steps_remaining > 0 && m_paused && m_steps_to_take == 0)
        {
            m_input_cv.wait(lock);
        }

        while (m_buffer.empty()) {
            if (!block)
                return 0;
            m_input_cv.wait(lock);
        }

        c = m_buffer.front();
        m_buffer.pop_front();
        m_keylog.push_back(c);

        if (m_replay_steps_remaining > 0) {
            --m_replay_steps_remaining;
            if (is_replay)
                *is_replay = true;
            if (!m_paused && m_replay_sleep)
                sleep = m_replay_sleep;
            if (m_steps_to_take > 0 && !(for_string && c != '\r' && c != ESCAPE))
                --m_steps_to_take;

            if (m_pause_at && m_pause_at == m_replay_steps_remaining)
            {
                m_paused = true;
                m_steps_to_take = 0;
            }
        }
    }

    if (sleep)
        Delay(sleep);

    return c;
}

void ReplayableInput::Flush()
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    if (m_replay_steps_remaining > 0)
        return;

    m_buffer.clear();
}

bool ReplayableInput::HasTypeahead()
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    return !m_buffer.empty();
}

void ReplayableInput::SaveGame(std::ostream & file)
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    std::copy(m_keylog.begin(), m_keylog.end(), std::ostreambuf_iterator<char>(file));
}

void ReplayableInput::RestoreGame(std::istream & file)
{
    m_buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    m_replay_steps_remaining = (int)m_buffer.size();

    std::string value;
    if (m_current_env->Get("replay_paused", &value) && value == "true") {
        m_paused = true;
    }
    if (m_current_env->Get("replay_pause_at", &value)) {
        m_pause_at = atoi(value.c_str());
    }
    if (m_current_env->Get("replay_step_delay", &value)) {
        m_replay_sleep = atoi(value.c_str());
    }
}

bool ReplayableInput::GetRenderText(std::string* text)
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    if (!m_replay_steps_remaining)
        return false;

    std::ostringstream ss;
    if (m_paused)
        ss << "Paused ";
    else
        ss << "Replay ";
    ss << m_replay_steps_remaining;
    *text = ss.str();
    return true;
}

void ReplayableInput::PauseReplay()
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    if (m_paused) {
        ++m_steps_to_take;
        if (m_buffer.front() == 'f' && m_options.emulate_ctrl_controls) {
            ++m_steps_to_take;
        }
    }
    m_paused = true;
    m_input_cv.notify_all();
}

void ReplayableInput::ResumeReplay()
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_paused = false;
    m_steps_to_take = 0;
    m_input_cv.notify_all();
}

void ReplayableInput::CancelReplay()
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_buffer.clear();
    m_replay_steps_remaining = 0;
    m_paused = false;
    m_steps_to_take = 0;
    m_input_cv.notify_all();
}

void ReplayableInput::DecreaseReplaySpeed()
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_replay_sleep *= 2;
    if (m_replay_sleep == 0)
        m_replay_sleep = 20;
}

void ReplayableInput::IncreaseReplaySpeed()
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_replay_sleep /= 2;
}

void ReplayableInput::SetMaxReplaySpeed()
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_replay_sleep = 0;
    m_paused = false;
    m_steps_to_take = 0;
    m_input_cv.notify_all();
}

void ReplayableInput::SetReplaySpeed(int n)
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_replay_sleep = n * 15;
    m_paused = false;
    m_steps_to_take = 0;
    m_input_cv.notify_all();
}

void ReplayableInput::QueueInput(const std::string & input)
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    std::copy(input.begin(), input.end(), std::back_inserter(m_buffer));
    m_input_cv.notify_all();
}

bool ReplayableInput::InReplay() const
{
    return m_replay_steps_remaining > 0;
}

GameConfig ReplayableInput::Options() const
{
    return m_options;
}
