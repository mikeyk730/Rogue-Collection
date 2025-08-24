Rog-O-Matic
===========================
[Rogomatic was created in the the early 1980s](https://www.cs.princeton.edu/~appel/papers/rogomatic.html) 
to play the Unix versions of Rogue. I've made changes to get it running against all versions,
but it may not perform as well on the PC versions, since it doesn't have any specific tactics for them.
It is capable of becoming a total winner, and some example runs are available [here](https://github.com/mikeyk730/Rogue-Collection/tree/main/test)

Running Rogomatic
========
Run Rogomatic by specifying the `--rogomatic` flag and specifying the game letter:

```
RogueCollection.exe --rogomatic [game_letter]
RetroRogueCollection.exe --rogomatic [game_letter]
```

I've added utility scripts `Rogomatic.bat` and `RetroRogomatic.bat` to make this easier for people without command line experience.
After selecting your version of Rogue, Rogomatic will then start to play automatically. Set `rogomatic_paused=true` in `rogue.opt` if you'd like it to start paused.

When using Rogomatic, 2 similar windows launch:
- The Rogue game (has the Rogue version in the title bar)
- The Rogomatic player interface (has Rog-O-Matic in the title bar)

Both display the map, but the Rogomatic window displays messages about what actions
it's performing, and allows the user to take control, pause the run, or view internal
information

Controls
========

Many of the original Rogue commands allow you to take control of Rogomatic
- Movement keys
- Search
- Go up/down stairs

## General Controls

| Command | Action
|---------|----------------------------------------------------------------------
| ?       | List all commands. Keep pressing ? to cycle through the pages
| t       | Pause/resume Rogomatic
| Enter   | Step through Rogomatic 1 turn at a time
| :       | Toggle chicken vs aggressive
| c       | Toggle cheating vs righteous
| Q       | Quit Rogomatic

## Debugging Commands

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
| ~       | Print Rogomatic version
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

Genes and Long Term Memory
====
Rogomatic's strategy is largely determined by hard-coded rules, but it uses a genetic algorithm to tweak
how likely it is to take certain actions

|Gene|Effect
|-|-
|Sr|Propensity for searching for traps
|Dr|Propensity for searching for doors
|Re|Propensity for resting
|Ar|Propensity for firing arrows
|Ex|Level*10 on which to experiment with items
|Rn|Propensity for retreating
|Wk|Propensity for waking things up
|Fd|Propensity for hoarding food (affects rings)

Rogomatic doesn't have any hard-coded stats about how powerful monsters are. It tracks combat statistics across all games,
and uses this long term memory to determine the danger (how likely the monsster is to hit, how much damage it's likely to deal).
The more games it plays, the more accurate it should be. On the other hand, behavior to deal with monster's special abilities
is hard-coded into the program.

Rogomatic Files
====
Rogomatic data files live in the `rlog` directory

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
The `ltm` file is updated after every game. It contains Rogomatic's long term memory about monsters.
This file is copied to `ltm.<seed>` at the begining of every game, so previous runs can be repeated

## snapshot.rgm
The `snapshot.rgm` contains all snapshots that we captured. Snapshots show a representation of the current screen
and a dump of the player's inventory. Snapshots can be taken manually during the run with `/`.
They are also taken automatically during unexpected scenarios for debugging, or during notable game
events like finding the amulet

Replaying a Rogomatic Run
====
Rogomatic is deterministic, so if it plays the same seed of Rogue, it will make the same decisions. 
You can replay a run by passing the seed and genes on the command line (find these in `rgmdelta`). For example:

    RogueCollection.exe b --rogomatic --seed 1755751405 --genes "22 37 35 46 39 18 13 88"

Rogomatic will load `ltm.<seed>` so it starts with the same memory as the original run.

Another useful tip for replaying: If you set `autosave=force` in `rogue.opt`, a .sav file will be created for every run.
Loading the .sav will simply replay the inputs, rather than running Rogomatic again. I've left Rogomatic running overnight,
and looked at `rgmdelta` to see which were worth watching.
Some sample winning runs are available [here](https://github.com/mikeyk730/Rogue-Collection/tree/main/test)!
