#include "game_config.h"
#include "environment.h"

TileConfig pc_tiles =          { "tiles.bmp",       78, 2 };
TileConfig atari_slime_tiles = { "atari.bmp",       78, 1 };
TileConfig atari_snake_tiles = { "atari_snake.bmp", 78, 1 };

TextConfig pc_text =         { "text.bmp",  { 256, 1 }, { 0x07 }, false };
TextConfig pc_colored_text = { "text.bmp",  { 256, 1 }, { 0x07 }, true  };
TextConfig alt_text =        { "16x16.bmp", { 16, 16 }, { 0x0F }, true  };
TextConfig boxy_text =       { "boxy.bmp",  { 16, 16 }, { 0x0F }, true  };

FontConfig placeholder = { "", 16 };
                                 //name          text_cfg          font_cfg      tile_cfg            unix    color  stndout animate
GraphicsConfig unix_gfx =        { "unix",       &pc_text,         &placeholder, 0,                  true,   false,  true,   false };
GraphicsConfig color_unix_gfx =  { "unix_color", &pc_colored_text, &placeholder, 0,                  true,   true,   true,   true  };
GraphicsConfig pc_bw_gfx =       { "pc_bw",      &pc_text,         &placeholder, 0,                  false,  false,  true,   true  };
GraphicsConfig pc_gfx =          { "pc",         &pc_colored_text, &placeholder, 0,                  false,  true,   true,   true  };
GraphicsConfig atari_slime_gfx = { "tiles",      &alt_text,        0,            &atari_slime_tiles, false,  false,  true,   false };
GraphicsConfig atari_snake_gfx = { "tiles",      &alt_text,        0,            &atari_snake_tiles, false,  false,  true,   false };
GraphicsConfig boxy_gfx =        { "boxy",       &boxy_text,       0,            0,                  false,  true,   true,   true  };


std::vector<GameConfig> s_options = {
    { "PC Rogue 1.48",    "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, atari_slime_gfx/*, boxy_gfx*/, unix_gfx } },
    { "PC Rogue 1.1",     "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, atari_snake_gfx/*, boxy_gfx*/, unix_gfx } },
    { "Unix Rogue 5.4.2", "Rogue_5_4_2.dll",   {80,25}, {80,24}, false, true,  { unix_gfx, pc_gfx, atari_snake_gfx/*, boxy_gfx*/ } },
    { "Unix Rogue 5.2.1", "Rogue_5_2_1.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, pc_gfx /*boxy_gfx*/ } },
    { "Unix Rogue 3.6.3", "Rogue_3_6_3.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, pc_gfx /*boxy_gfx*/ } },
};

GameConfig GetGameConfig(const std::string& name)
{
    for (int i = 0; i < s_options.size(); ++i){
        if (s_options[i].name == name) {
            return s_options[i];
        }
    }
    throw std::runtime_error("Unknown game " + name);
}

void SetFontFile(const std::string & filename)
{
    placeholder.fontfile = filename;
}

void SetFontSize(int size)
{
    placeholder.size = size;
}

void InitGameConfig(Environment* env)
{
    std::string value;
    if (env->Get("font", &value)) {
        SetFontFile(value);
        if (env->Get("font_size", &value)) {
            int size = atoi(value.c_str());
            if (size) {
                SetFontSize(size);
            }
        }
    }
}
