#pragma once
#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <windows.h>
#include "input_interface.h"

struct StreamInput : public InputInterface
{
    StreamInput(std::istream& in, InputInterface* backup);

    virtual char GetNextChar();
    virtual std::string GetNextString(int size);

    virtual void Serialize(std::ostream& out);

    void OnStreamEnd();

    void PausePlayback();
    void StepPlayback();
    void ResumePlayback();
    void CancelPlayback();

private:
    std::unique_ptr<InputInterface> m_backup;
    std::istream& m_stream;
    
    HANDLE m_thread;
    std::mutex m_mutex;
    std::condition_variable m_step_cv;
    bool m_paused = false; //accessed from multiple threads
    int m_steps = 0;       //accessed from multiple threads
};
