CC=g++
SRCROOT=../..
CFLAGS=-Wall -Werror -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses -I$(SRCROOT)/RogueVersions/Rogue_PC_Core
DEPS = *.h
OBJ = curses_input.o  curses_output.o  main.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rogue-pc-1-48: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
