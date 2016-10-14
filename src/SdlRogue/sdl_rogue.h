#pragma once
#include <memory>
#include <vector>
#include <display_interface.h>
#include <input_interface.h>

struct Region;
struct Coord;

struct TileConfig
{
    std::string filename;
    int count;
    int states;
};

struct TextConfig
{
    std::string filename;
    std::vector<int> colors;
};

struct Options
{
    bool emulate_alt_controls = false;
    bool use_unix_gfx = false;
    bool use_colors = true;
};

struct SdlRogue : public DisplayInterface, public InputInterface
{
    SdlRogue(const TextConfig& text, TileConfig* tiles, Options options);
    ~SdlRogue();

    void Run();
    void Quit();

    //display interface
    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(uint32_t* buf) override;
    virtual void UpdateRegion(uint32_t* info, Region rect) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;

    //input interface
    virtual char GetChar(bool block) override;
    virtual std::string GetString(int size) override;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
