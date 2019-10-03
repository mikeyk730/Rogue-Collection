#include "game_config.h"
#include "environment.h"

namespace
{
    TileConfig tilemap_v1 = { "tilemap_v1.bmp", 61, 2 };
    TileConfig tilemap_v2 = { "tilemap_v2.bmp", 61, 2 };
    TileConfig tilemap_v3 = { "tilemap_v3.bmp", 61, 2 };
    TileConfig tilemap_v4 = { "tilemap_v4.bmp", 61, 2 };

    TextConfig tilemap_text =  { "tilemap_text.bmp", { 16, 16 }, { 0x07 }, true };
    TextConfig boxy_text =     { "boxy.bmp",         { 16, 16 }, { 0x07 }, true };

    //                                 name      text_cfg        fnt       tile_cfg      unix    color   stndout animate
    GraphicsConfig unix_gfx =        { "unix",   nullptr,        nullptr,  nullptr,      true,   false,  true,   false };
    GraphicsConfig pc_gfx =          { "pc",     nullptr,        nullptr,  nullptr,      false,  true,   true,   true  };
    GraphicsConfig tilemap_v1_gfx =  { "tiles",  &tilemap_text,  nullptr,  &tilemap_v1,  false,  true,   true,   false };
    GraphicsConfig tilemap_v2_gfx =  { "tiles",  &tilemap_text,  nullptr,  &tilemap_v2,  false,  true,   true,   false };
    GraphicsConfig tilemap_v3_gfx =  { "tiles",  &tilemap_text,  nullptr,  &tilemap_v3,  false,  true,   true,   false };
    GraphicsConfig tilemap_v4_gfx =  { "tiles",  &tilemap_text,  nullptr,  &tilemap_v4,  false,  true,   true,   false };
    GraphicsConfig boxy_gfx =        { "boxy",   &boxy_text,     nullptr,  nullptr,      false,  true,   true,   true  };

#ifdef _WIN32 //todo:mdk
    const char* Rogomatic     = "Rogomatic_Player.dll";
    const char* Rogue_PC_1_48 = "Rogue_PC_1_48.dll";
    const char* Rogue_5_4_2   = "Rogue_5_4_2.dll";
    const char* Rogue_5_3     = "Rogue_5_3.dll";
    const char* Rogue_5_2_1   = "Rogue_5_2_1.dll";
    const char* Rogue_3_6_3   = "Rogue_3_6_3.dll";
#else
    const char* Rogomatic     = "lib-rogomatic-player.so";
    const char* Rogue_PC_1_48 = "lib-rogue-pc-1-48.so";
    const char* Rogue_5_4_2   = "lib-rogue-5-4-2.so";
    const char* Rogue_5_3     = "lib-rogue-5-3.so";
    const char* Rogue_5_2_1   = "lib-rogue-5-2-1.so";
    const char* Rogue_3_6_3   = "lib-rogue-3-6-3.so";
#endif
}

std::vector<GameConfig> s_options = {
    { "PC Rogue 1.48",    Rogue_PC_1_48, {80,25}, {40,25}, true,  false, { pc_gfx, tilemap_v4_gfx, boxy_gfx, unix_gfx }, false },
    { "PC Rogue 1.1",     Rogue_PC_1_48, {80,25}, {40,25}, true,  false, { pc_gfx, tilemap_v3_gfx, boxy_gfx, unix_gfx }, false },
    { "Unix Rogue 5.4.2", Rogue_5_4_2,   {80,24}, {80,24}, false, true,  { unix_gfx, pc_gfx, tilemap_v2_gfx, boxy_gfx }, true },
    { "Unix Rogue 5.3",   Rogue_5_3,     {80,24}, {80,24}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, false },
    { "Unix Rogue 5.2.1", Rogue_5_2_1,   {80,24}, {70,22}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, true },
    { "Unix Rogue 3.6.3", Rogue_3_6_3,   {80,24}, {70,22}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, true },
    { "Rog-o-matic",      Rogomatic,     {80,24}, {80,24}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, false },
};

GameConfig GetGameConfig(int i)
{
    return s_options[i];
}

int GetGameIndex(const std::string& name)
{
    for (size_t i = 0; i < s_options.size(); ++i){
        if (s_options[i].name == name) {
            return i;
        }
    }

    return -1;
}
