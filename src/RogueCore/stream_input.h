#pragma once
#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include "input_interface.h"

struct StreamInput : public InputInterfaceEx
{
    StreamInput(std::unique_ptr<std::istream> in, int version, bool start_paused);
    ~StreamInput();

    virtual bool HasMoreInput() override;
    virtual char GetNextChar(bool *is_replay) override;
    virtual std::string GetNextString(int size) override;

    virtual void Serialize(std::ostream& out);
    
    void OnReplayEnd(const std::function<void()>& handler);
    void OnFastPlayChanged(const std::function<void(bool)>& handler);

private:
    void OnStreamEnd();
    char ReadChar();
    std::string ReadString();

    void NotifyReplayEnd();
    void NotifyFastPlayChanged(bool enable);

    std::unique_ptr<std::istream> m_stream;
    char m_version;
    std::function<void()> m_on_end;
    std::function<void(bool)> m_on_fast_play;
    std::deque<unsigned char> m_typeahead;

public:
    struct ThreadData
    {
        void PausePlayback();
        void StepPlayback();
        void ResumePlayback();
        void CancelPlayback();

        void OnEmptyStream();

        std::mutex m_mutex;
        std::condition_variable m_step_cv;

        bool m_paused = false;
        bool m_canceled = false;
        int m_steps = 0;
        bool m_stream_empty = false;
        int m_throttle = 0;
    };

private:
    std::shared_ptr<ThreadData> m_shared_data;
};
