#pragma once
#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "input_interface.h"

struct StreamInput : public InputInterface
{
    StreamInput(std::istream& in, int version);
    ~StreamInput();

    virtual bool HasMoreInput();
    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);

private:
    void OnStreamEnd();
    char ReadCharA();
    std::string ReadStringA();
    std::string ReadStringB();

    std::istream& m_stream;
    char m_version;

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
    };

private:
    std::shared_ptr<ThreadData> m_shared_data;
};
