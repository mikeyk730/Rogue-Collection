std::vector<GameConfig> s_options = {
    { "PC Rogue 1.48",    Rogue_PC_1_48, {80,25}, {40,25}, true,  false, { pc_gfx, tilemap_v4_gfx, boxy_gfx, unix_gfx }, false },
    { "PC Rogue 1.1",     Rogue_PC_1_48, {80,25}, {40,25}, true,  false, { pc_gfx, tilemap_v3_gfx, boxy_gfx, unix_gfx }, false },
    { "Unix Rogue 5.4.2", Rogue_5_4_2,   {80,24}, {80,24}, false, true,  { unix_gfx, pc_gfx, tilemap_v2_gfx, boxy_gfx }, true  },
    { "Unix Rogue 5.3",   Rogue_5_3,     {80,24}, {80,24}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, false },
    { "Unix Rogue 5.2.1", Rogue_5_2_1,   {80,24}, {70,22}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, true  },
    { "Unix Rogue 3.6.3", Rogue_3_6_3,   {80,24}, {70,22}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, true  },
    { "Rog-o-matic",      Rogomatic,     {80,24}, {80,24}, true,  true,  { unix_gfx, pc_gfx, tilemap_v1_gfx, boxy_gfx }, false },
};
