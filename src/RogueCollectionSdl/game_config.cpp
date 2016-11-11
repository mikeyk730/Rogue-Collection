#include "game_config.h"
#include "environment.h"

TileConfig pc_tiles =   { "tiles.bmp",       78, 2 };
TileConfig tilemap_v1 = { "tilemap_v1.bmp",  78, 1 };
TileConfig tilemap_v2 = { "tilemap_v2.bmp",  78, 1 };
TileConfig tilemap_v3 = { "tilemap_v3.bmp",  78, 1 };

TextConfig pc_text =         { "text.bmp",  { 256, 1 }, { 0x07 }, false };
TextConfig pc_colored_text = { "text.bmp",  { 256, 1 }, { 0x07 }, true  };
TextConfig alt_text =        { "tilemap_text.bmp", { 16, 16 }, { 0x0F }, true  };
TextConfig boxy_text =       { "boxy.bmp",  { 16, 16 }, { 0x0F }, true  };

FontConfig placeholder = { "", 16 };
                                 //name          text_cfg          font_cfg      tile_cfg      unix    color  stndout animate     
GraphicsConfig unix_gfx =        { "unix",       &pc_text,         &placeholder, 0,            true,   false,  false,  false };
GraphicsConfig color_unix_gfx =  { "unix_color", &pc_colored_text, &placeholder, 0,            true,   true,   true,   true  };
GraphicsConfig pc_gfx =          { "pc",         &pc_colored_text, &placeholder, 0,            false,  true,   true,   true  };
GraphicsConfig tilemap_v1_gfx =  { "tiles",      &alt_text,        0,            &tilemap_v1,  false,  false,  true,   false };
GraphicsConfig tilemap_v2_gfx =  { "tiles",      &alt_text,        0,            &tilemap_v2,  false,  false,  true,   false };
GraphicsConfig tilemap_v3_gfx =  { "tiles",      &alt_text,        0,            &tilemap_v3,  false,  false,  true,   false };
GraphicsConfig boxy_gfx =        { "boxy",       &boxy_text,       0,            0,            false,  true,   true,   true  };


std::vector<GameConfig> s_options = {
    { "PC Rogue 1.48",    "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, tilemap_v3_gfx, boxy_gfx, unix_gfx, color_unix_gfx } },
    { "PC Rogue 1.1",     "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, tilemap_v2_gfx, boxy_gfx, unix_gfx, color_unix_gfx } },
    { "Unix Rogue 5.4.2", "Rogue_5_4_2.dll",   {80,25}, {80,24}, false, true,  { unix_gfx, color_unix_gfx, pc_gfx, tilemap_v2_gfx, boxy_gfx } },
    { "Unix Rogue 5.2.1", "Rogue_5_2_1.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, color_unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx } },
    { "Unix Rogue 3.6.3", "Rogue_3_6_3.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, color_unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx } },
};

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
