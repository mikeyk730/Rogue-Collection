#pragma once
#include <memory>
#include <vector>
#include "../Shared/display_interface.h"
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
    Coord layout;
    std::vector<int> colors;
};

struct GraphicsConfig
{
    TextConfig* text_cfg;
    TileConfig* tile_cfg;
    bool use_unix_gfx;
    bool use_colors;
};

struct Options
{
    std::string name;
    std::string dll_name;
    bool emulate_ctrl_controls;
    bool scroll_lock_wake;
    std::vector<GraphicsConfig> gfx_options;
};

struct SdlRogue : public DisplayInterface, public InputInterface
{
    SdlRogue(SDL_Window* window, SDL_Renderer* renderer, const Options& options);
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

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
