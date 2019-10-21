#pragma once
#include <display_interface.h>

struct PipeOutput : public DisplayInterface
{
    PipeOutput(int pipe_fd);

    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(uint32_t* info, char* dirty) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;
    virtual void PlaySound(const std::string& id) override;
    virtual void DisplayMessage(const std::string& message) override;

private:
    void WriteRogomaticPosition(Coord pos);
    void WriteRogomaticScreen(uint32_t* data, char* dirty);
    int TotalChars() const;

    int pipe_fd_;
    Coord dimensions_;
};
