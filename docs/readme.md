Retro Rogue Collection v3.0
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
| Ctrl+direction | All versions        | Run until you're beside something of interest. (If Scroll Lock is on, running is the default behavior)
| .              | All versions        | Rest
| z+direction    | All versions        | Zap in the given direction
| m+direction    | PC 1.48, Unix 5.4.2 | Move onto an object without picking it up
| Ctrl+R         | All versions        | Repeat last message
| Ctrl+E         | All versions        | Redraw screen
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
| *       | Show item probability

Configuration
=============
The `rogue.opt` file contains many different configuration options.  Open it in a text editor and modify it to your liking.  In addition, some options can be set on the command line:

~~~
Usage: RetroRogueCollection.exe [flags] [savefile|game_letter|game_name]

flags:        -o|--optfile <file>  Use the given Rogue option file instead of rogue.opt.
              -n|--small-screen    Run with the smallest screen that the game supports.
              -p|--paused          Start replay paused.
              --pause-at <n>       Pause replay with <n> steps remaining.
              --default-settings   Run with the default profile.
              --profile <prof>     Run with the given profile.
              --rogomatic          Spawn Rog-O-Matic to play the game.
              --verbose            Print additional information such as profiles and settings.

savefile:     Path to a save file (e.g. "rogue.sav").
game_letter:  Letter from the game select menu (e.g. "b").
game_name:    Name from the game select menu (e.g. "PC Rogue 1.1").
~~~

Strategy
=======
PC Versions
-------
- Try to identify `scare monster` scrolls before picking them up
    - Since monsters won't pass over `scare monster` scrolls, try to lure a monster over the scroll. If the monster avoids it, you know it's a `scare monster`. These are too useful to waste by reading them
- Before reading unknown scrolls:
    - Equip some good armor (in case it's `enchant armor`)
    - Equip a good weapon (in case it's `enchant weapon` or `vorpalize weapon`)
    - Equip/unequip all rings to find the cursed ones (in case it's a `remove curse`)
- Use bad potions against monsters:
    - Always throw potions at monsters in v1.48, since they'll always hit. In v1.1 monsters will get a saving throw, so you'll need more luck
    - In v1.1 potions can be wielded and never break, so you can reuse them to affect
    - Confusion and blindness potions will confuse the monster
    - Paralysis potions will freeze the monster in place, preventing it from chasing you
- Know the useful tactics for getting through the later levels
    - Avoid monsters in the later levels if you can. Read a `scroll of magic mapping` to find the stairs and get out ASAP
    - Prevent monsters from attacking:
      - Drop a `scroll of scare monster`. Enemies won't attack while you're standing on it 
    - Freeze the monster so you can run away:
      - Read a `scroll of hold monster` to freeze all monsters in a 3x3 area around the player
      - Throw a `potion of paralysis` at a monster to freeze it in place
    - Teleport in an emergency:
      - Read the `scroll of teleportation` if you're desparate to jump to a new location
      - Zap the monster with a `teleport away` stick to send it somewhere else
    - Transform the monster:
      - Zap the monster with a `polymorph` stick to turn a dangeous monster into something (hopefully) less powerful
    - Take extra actions on your turn:
      - Quaff a `potion of haste self` to get 2-3 actions per turn. This allows you to get extra hits on the monster or run away
    - Confuse the monster:
      - Throw a `potion of confusion` or `potion of blindness` at a monster to confuse it.
      - Read a `scroll of monster confusion` and your next hit will confuse the monster. It takes a turn to read the scroll, and then you still need to land a hit
- Know which items are not generally useful:
    - Scrolls: sleep, blank paper, create monster, aggravate monsters
    - Potions: poison, thirst quenching

Credits
=======
Rogue
-----
I do not own the rights to the original Rogue games.

- Copyright (C) 1981 Michael Toy, Ken Arnold, and Glenn Wichman
- Copyright (C) 1983 Jon Lane (A.I. Design update for the IBM PC)
- Copyright (C) 1985 Epyx

Thanks to the [Roguelike Restoration Project](https://github.com/RoguelikeRestorationProject) and the [coredumpcentral.org](http://www.coredumpcentral.org) fork for updating Unix versions to modern environments.

Rogue-O-Matic
-------------
Original version:
- Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin

Restoration and updates:
- Copyright (C) 2008 by Anthony Molinaro
- Copyright (C) 2011-2015 by ant <ant at anthive dot com>

Retro Effects
-------------
The retro effects were adapted from [cool-retro-term](https://github.com/Swordfish90/cool-retro-term).

- Copyright (C) 2013 Filippo Scognamiglio

Tilesets
--------
Tilesets were adapted from [DawnHack](http://dragondeplatino.deviantart.com/art/DawnHack-NetHack-3-4-3-UnNetHack-5-1-0-416312313) by _DragonDePlatino_, used under [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/)

Alternate tilesets by _Rogue Yun_ and _Marble Dice_ from the [Dwarf Fortress Tileset Repository](http://dwarffortresswiki.org/index.php/Tileset_repository)

Sound
-----
Sound effects from Classic Rogue v1.51 by _Donnie Russell_

Item sound effect by [_Seidhepriest_](https://www.freesound.org/people/Seidhepriest/), used under [CC BY-NC 3.0](https://creativecommons.org/licenses/by-nc/3.0/)

Fonts
-----
IBM PC fonts from [The Ultimate Oldschool PC Font Pack](http://int10h.org/oldschool-pc-fonts/) by _VileR_, used under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

License
=======
The _Retro Rogue Collection_ engine is licensed under the GNU General Public License Version 3.  See `license\gpl-3.0.txt` for more information.  The original Rogue and Rog-O-Matic code belongs to the original copyright holders.  See `license\unix-rogue.txt` for more information regarding the Unix versions.
