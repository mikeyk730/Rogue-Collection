CC=g++
SRCROOT=..
LIBROOT=../../lib
CFLAGS=-Wall -Werror -I/usr/include/SDL2 -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses -I$(LIBROOT)/SDL2_ttf-2.0.14/include -I$(LIBROOT)/SDL2_image-2.0.1/include -I$(LIBROOT)/NativeFileDialog/src/include
DEPS = *.h
OBJ = args.o game_select.o sdl_display.o text_provider.o dos_to_unicode.o key_utility.o sdl_input.o tile_provider.o environment.o main.o sdl_rogue.o utility.o game_config.o replayable_input.o sdl_utility.o window_sizer.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rogue-collection-sdl: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
