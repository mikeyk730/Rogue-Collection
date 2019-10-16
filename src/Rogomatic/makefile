CC       = gcc
SHELL    = /bin/sh
SRCROOT  = ..

INCLUDES = -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses
DEFINES  = -DROGUE_COLLECTION #-DROGOMATIC_PROTOCOL_DEBUGGING
CFLAGS   = -w -shared -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -s

TARGET   = lib-rogomatic-player.so
SOURCES  = $(shell echo *.c)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.c=.o) pc_gfx.o

BUILDROOT= $(SRCROOT)/../build/release
CFLAGS  += -DDEBUG -g
BUILDROOT= $(SRCROOT)/../build/debug
LIBS     = -Wl,--whole-archive $(BUILDROOT)/lib-my-curses.a -Wl,--no-whole-archive

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS) $(LIBS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) $(DEFINES)

pc_gfx.o: ../RogueVersions/pc_gfx.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) $(DEFINES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)