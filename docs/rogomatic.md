Rog-O-Matic
===========================

```
RogueCollection.exe --rogomatic [game_letter]
RetroRogueCollection.exe --rogomatic [game_letter]
```

When using Rog-O-Matic, 2 similar windows launch:
- The Rogue game
- The Rog-O-Matic interface

If you didn't supply the game version on the command line, you will need to select it
from the menu on the main Rogue window. Rog-O-Matic will start to play automatically,
and you can type commands on the Rog-O-Matic window

Controls
========

Many of the original Rogue commands allow you to take control of Rog-O-Matic
- Movement keys
- Search
- Go up/down stairs

## General Controls

| Command | Action
|---------|----------------------------------------------------------------------
| ?       | List all commands. Keep pressing ? to cycle through the pages
| t       | Pause/resume Rog-O-Matic
| Enter   | Step through Rog-O-Matic 1 turn at a time
| :       | Toggle chicken vs aggressive
| c       | Toggle cheating vs righteous
| Q       | Quit Rog-O-Matic

## Debugging Controls

| Command | Action
|---------|----------------------------------------------------------------------
| d       | Cycle through debugger breakpoint options
| /       | Take a snapshot. This dumps the level and stats to a file on disk
| m       | Show monster table
| (       | Show database
| i       | Show inventory
| r       | Reset inventory
| e       | Toggle logging to file
| @       | Show player coordinates
| M       | Show maze doors
| )       | Mark cycles
| A       | Increment attempt counter
| G       | Show genome
| [       | Print battle info
| -       | Print status
| ~       | Print Rog-O-Matic version
| \|       | Show time stats
| !       | Show stuff on map
| @       | Show monsters on map
| #       | Show walls
| %       | Show armor info
| ]       | Show rustproof armor info
| =       | Show ring info
| $       | Show weapon info
| ^       | Show bow info
| &       | Show object count
| *       | Toggle blinded vs sighted
| C       | Toggle cosmic vs boring

Rog-O-Matic Files
====
Rog-O-Matic data files live in the `rlog` directory

## rgmdelta
The `rgmdelta` file has a summary of all games
```
Date         Player      Gold Killed By          Lvl HP   Str  AC Exp       Game    Seed          Genes
```
```
Aug 21, 2025 rogomati    2820 ur-vile            16  62   13  10  9/2071    1277    1755751260    24 37 48 46 44 28 13 65
Aug 21, 2025 rogomati     599 starvation          6  55   14   4  7/377     1278    1755751314    12 37 43 46 44 18 25 88
Aug 21, 2025 rogomati     883 centaur             8  50   13   7  7/345     1279    1755751359    17 37 43 60 40 52 12 67
Aug 21, 2025 rogomati    2665 troll              12  46   16   8  8/1012    1280    1755751405    22 37 35 46 39 18 13 88
```

## ltm
The `ltm` file is updated after every game. It contains Rog-O-Matic's long term memory about monsters.
This file is copied to `ltm.<seed>` at the begining of every game.

## snapshot.rgm
The `snapshot.rgm` contains all snapshots that we captured. Snapshots can be taken manually with `/`.
They are also taken automatically during unexpected scenarios for debugging, or during notable game
events like finding the amulet

Replaying a Rog-O-Matic Run
====
You can replay a run by passing the seed and genes on the command line. For example:

    RogueCollection.exe b --rogomatic --seed 1755751405 --genes "22 37 35 46 39 18 13 88"

Rog-O-Matic will load `ltm.<seed>` so it starts with the same memory as the original run
