#pragma once
#ifdef _WIN32
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
