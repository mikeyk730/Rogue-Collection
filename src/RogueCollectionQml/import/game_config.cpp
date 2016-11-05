#include "game_config.h"
#include "environment.h"

TileConfig atari_slime_tiles = { "atari.bmp",       78, 1 };
TileConfig atari_snake_tiles = { "atari_snake.bmp", 78, 1 };

TextConfig alt_text =  { "16x16.bmp", { 16, 16 }, { 0x0F }, true };
TextConfig boxy_text = { "boxy.bmp",  { 16, 16 }, { 0x0F }, true };

                                 //name      text_cfg    fnt  tile_cfg             unix    color  stndout animate
GraphicsConfig unix_gfx =        { "unix",   0,           0,  0,                   true,   false,  true,   false };
GraphicsConfig pc_gfx =          { "pc",     0,           0,  0,                   false,  true,   true,   true  };
GraphicsConfig atari_slime_gfx = { "tiles",  &alt_text,   0,  &atari_slime_tiles,  false,  false,  true,   false };
GraphicsConfig atari_snake_gfx = { "tiles",  &alt_text,   0,  &atari_snake_tiles,  false,  false,  true,   false };
GraphicsConfig boxy_gfx =        { "boxy",   &boxy_text,  0,  0,                   false,  true,   true,   true  };

std::vector<GameConfig> s_options = {
    { "PC Rogue 1.48",    "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, atari_slime_gfx, boxy_gfx, unix_gfx } },
    { "PC Rogue 1.1",     "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, atari_snake_gfx, boxy_gfx, unix_gfx } },
    { "Unix Rogue 5.4.2", "Rogue_5_4_2.dll",   {80,25}, {80,24}, false, true,  { unix_gfx, pc_gfx, atari_snake_gfx, boxy_gfx } },
    { "Unix Rogue 5.2.1", "Rogue_5_2_1.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, pc_gfx, boxy_gfx } },
    { "Unix Rogue 3.6.3", "Rogue_3_6_3.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, pc_gfx, boxy_gfx } },
};

GameConfig GetGameConfig(int i)
{
    return s_options[i];
}

int GetGameIndex(const std::string& name)
{
    for (int i = 0; i < s_options.size(); ++i){
        if (s_options[i].name == name) {
            return i;
        }
    }
    return -1;
}
