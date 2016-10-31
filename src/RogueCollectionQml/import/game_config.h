#pragma once
#include <string>
#include <vector>
#include <coord.h>

struct Environment;

struct FontConfig
{
    std::string fontfile;
    int size;
};

struct TextConfig
{
    std::string imagefile;
    Coord layout;
    std::vector<int> colors;
    bool generate_colors;
};

struct TileConfig
{
    std::string filename;
    int count;
    int states;
};

struct GraphicsConfig
{
    std::string name;
    TextConfig* text;
    FontConfig* font;
    TileConfig* tiles;
    bool use_unix_gfx;
    bool use_colors;
    bool use_standout;
    bool animate;
};

struct GameConfig
{
    std::string name;
    std::string dll_name;
    Coord screen;
    Coord small_screen;
    bool emulate_ctrl_controls;
    bool is_unix;
    std::vector<GraphicsConfig> gfx_options;
};

const int kWindowWidth = 640;
const int kWindowHeight = 400;

void InitGameConfig(Environment* env);
extern std::vector<GameConfig> s_options;
