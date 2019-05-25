CC=g++
SRCROOT=../..
CFLAGS=-I$(SRCROOT)/Shared
#-I/usr/include/SDL2 -I../MyCurses -I../../lib/SDL2_ttf-2.0.14/include -I../../lib/SDL2_image-2.0.1/include -I../../lib/NativeFileDialog/src/include
DEPS = *.h
OBJ = agent.o amulet.o armor.o captured_input.o combo_input.o command.o commands.o daemon.o daemons.o extern.o fakedos.o fight.o food.o game_state.o gold.o hero.o io.o item_category.o item.o level.o list.o mach_dep.o main.o maze.o misc.o monster.o monsters.o move.o pack.o passages.o potions.o random.o rings.o rip.o room.o rooms.o save.o screen_output.o scrolls.o slime.o sticks.o stream_input.o strings.o things.o weapons.o wizard.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rogue-pc-core: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
