#pragma once
#include <memory>
#include <vector>
#include <map>
#include <display_interface.h>
#include <input_interface.h>

struct Region;
struct Coord;
struct Environment;

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
    std::string name;
    TextConfig* text_cfg;
    TileConfig* tile_cfg;
    bool use_unix_gfx;
    bool use_colors;
};

struct Options
{
    std::string name;
    std::string dll_name;
    Coord screen;
    Coord small_screen;
    bool emulate_ctrl_controls;
    bool is_unix;
    std::vector<GraphicsConfig> gfx_options;
};

struct TextProvider
{
    TextProvider(const TextConfig& config, SDL_Renderer* renderer);
    ~TextProvider();
    Coord dimensions() const;
    void GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect);

private:
    int TextProvider::get_text_index(unsigned short attr);
    SDL_Rect TextProvider::get_text_rect(unsigned char ch, int i);

    TextConfig m_cfg;
    SDL_Texture* m_text = 0;
    Coord m_text_dimensions = { 0, 0 };
    std::map<int, int> m_attr_index;
};


extern std::vector<Options> s_options;

struct SdlRogue : public DisplayInterface, public InputInterface
{
    SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> env, int index);
    SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> env, const std::string& filename);
    ~SdlRogue();

    void Run();
    void Quit();

    Environment* GameEnv() const;
    Options options() const;

    //display interface
    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(uint32_t* buf) override;
    virtual void UpdateRegion(uint32_t* info, Region rect) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;

    //input interface
    virtual char GetChar(bool block, bool for_string, bool *is_replay) override;
    virtual void Flush() override;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
