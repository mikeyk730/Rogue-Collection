#
# Make file for building Rogue
# Michael Toy, AI Design, May 1984
# :bk=0
#
# NOTE: If new files are added or old files removed from these
#      object lists, you must modify the file objs as well.
#
#      To disbale stack overflow checking (thus getting a smidge more
#      speed.  Don't load in csav.o
#
OBJS=armor.o new_leve.o command.o mach_dep.o croot.o main.o rip.o save.o \
       chase.o curses.o daemon.o daemons.o env.o extern.o fakedos.o fight.o \
       init.o io.o list.o maze.o misc.o monsters.o move.o pack.o passages.o \
       potions.o rings.o rooms.o scrolls.o sticks.o strings.o zoom.o things.o \
       weapons.o wizard.o dos.o begin.o fio.o csav.o slime.o sbrk.o load.o \
       protect.o

CFLAGS=+C -E140

rogue.exe: $(OBJS)
       ln -f objs -o rogue.exe -lc

jr: rogue.exe
       copy rogue.exe a:

waste:
       masm waste,d:waste,,,
       link d:waste,d:,,,
       exe2bin d:waste c:waste.com
       rm d:waste.obj d:waste.exe
backup:
       copy *.c a:
       copy *.asm a:
       copy *.h a:
       copy makefile a:
       copy objs a:
