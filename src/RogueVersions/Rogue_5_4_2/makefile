CC       = gcc
SHELL    = /bin/sh
SRCROOT  = ../..
BUILDROOT= $(SRCROOT)/../build/

INCLUDES = -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses
DEFINES  = -DROGUE_COLLECTION -DUSE_PC_STYLE -DSCOREFILE=\"rogue54.scr\" -DALLSCORES -DMASTER
CFLAGS   = -w -shared -fPIC
LIBS     = -Wl,--whole-archive $(BUILDROOT)/lib-my-curses.a -Wl,--no-whole-archive

TARGET   = lib-rogue-5-4-2.so
SOURCES  = $(shell echo *.c)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS) $(LIBS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) $(DEFINES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)