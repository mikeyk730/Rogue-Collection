Retro Rogue Collection v2.0
===========================
https://github.com/mikeyk730/Rogue-Collection

Copyright (C) 2016 Mike Kaminski

For the most recent version of this file, visit https://github.com/mikeyk730/Rogue-Collection/blob/master/docs/readme.md

Executables
===========
* `RetroRogueCollection.exe` is the fully featured program that includes all retro graphic effects.
* `RogueCollection.exe` is a lightweight version that has simpler graphics and lower memory usage.

Controls
========
Most controls have not been modified.  Use '?' in game to see a list of controls for that specific game.  The controls below have been tweaked for consistency between versions:

| Control        | Versions            | Action
|----------------|---------------------|----------------------------------------------------------------------
| Ctrl+direction | All versions        | Run until you're beside something of interest
| .              | All versions        | Rest
| z+direction    | All versions        | Zap in the given direction
| m+direction    | PC 1.48, Unix 5.4.2 | Move onto an object without picking it up
| Ctrl+R         | All versions        | Repeat last message
| Alt+number     | All versions        | Resize the window to the given scale (e.g. Alt+3 sets window to 300%)

Replay Mode
-----------
When a saved game is being replayed, the following controls can be used:

| Control  | Action
|----------|---------------------------------------------------
| Space    | Pause replay.  If paused, advance one step
| Enter    | Resume replay
| 0-9      | Resume replay at given speed (0=fastest 9=slowest)
| Escape   | Cancel replay

Wizard Mode
-----------
Wizard mode is used for debugging or cheating.  Using it disqualifies your score from the Top 10.  Different versions support different commands, but the master list is below:

| Control | Action
|---------|-----------------------------------------------------
| Ctrl+P  | Toggle wizard mode (password is 'y')
| Ctrl+W  | Identify objects
| Ctrl+E  | Print food remaining
| Ctrl+R  | Raise experience level
| Ctrl+T  | Teleport
| Ctrl+I  | Print items on level
| Ctrl+A  | Go up one level
| Ctrl+D  | Go down one level
| Ctrl+F  | Show level map
| Ctrl+G  | Equip player with good weapon, armor, and experience
| Ctrl+X  | Detect monsters
| Ctrl+C  | Show passages
| Ctrl+O  | Toggle additional powers
| ~       | Recharge a wand or staff
| $       | Print number of items in pack
| &#124;  | Print coordinates of player
| C       | Create object

Configuration
=============
The `rogue.opt` file contains many different configuration options.  Open it in a text editor and modify it to your liking.  In addition, some options can be set on the command line:

~~~
Usage: RetroRogueCollection.exe [flags] [savefile|game_letter|game_name]

flags:        -h|--help            Print this help.
              -o|--optfile <file>  Use the given Rogue option file instead of rogue.opt.
              -n|--small-screen    Run with the smallest screen that the game supports.
              -p|--paused          Start replay paused.
              --pause-at <n>       Pause replay with <n> steps remaining.
              --default-settings   Run with the default profile.
              --profile <prof>     Run with the given profile.
              --fullscreen         Run in fullscreen.
              --verbose            Print additional information such as profiles and settings.
              
savefile:     Path to a save file (e.g. "rogue.sav").
game_letter:  Letter from the game select menu (e.g. "b").
game_name:    Name from the game select menu (e.g. "PC Rogue 1.1").
~~~

Credits
=======
Rogue
-----
I do not own the rights to the original Rogue games.

- Copyright (C) 1981 Michael Toy, Ken Arnold, and Glenn Wichman
- Copyright (C) 1983 Jon Lane (A.I. Design update for the IBM PC)
- Copyright (C) 1985 Epyx

Thanks to the [Roguelike Restoration Project](https://github.com/RoguelikeRestorationProject) and the [coredumpcentral.org](http://www.coredumpcentral.org) fork for updating Unix versions to modern environments.

Retro Effects
-------------
The retro effects were adapted from [cool-retro-term](https://github.com/Swordfish90/cool-retro-term).

- Copyright (C) 2013 Filippo Scognamiglio

Fonts
-----
IBM PC fonts come from [_The Ultimate Oldschool PC Font Pack_](http://int10h.org/oldschool-pc-fonts/) by _VileR_

Tilesets
--------
Tilesets were adapted from [DawnHack](http://dragondeplatino.deviantart.com/art/DawnHack-NetHack-3-4-3-UnNetHack-5-1-0-416312313) by _DragonDePlatino_

Alternate tilesets by _Rogue Yun_ and _Marble Dice_ from the [Dwarf Fortress Tileset Repository](http://dwarffortresswiki.org/index.php/Tileset_repository)

License
=======
The _Retro Rogue Collection_ engine is licensed under the GNU General Public License Version 3.  See `license\gpl-3.0.txt` for more information.  The original Rogue game code belongs to the original copyright holders.  See `license\unix-rogue.txt` for more information regarding the Unix versions.
