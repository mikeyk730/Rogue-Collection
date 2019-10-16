CC       = g++
SHELL    = /bin/sh
SRCROOT  = ../..

INCLUDES = -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses -I$(SRCROOT)/RogueVersions/Rogue_PC_Core
CFLAGS   = -Wall -Werror -pedantic -shared -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -s

TARGET   = lib-rogue-pc-1-48.so
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

BUILDROOT= $(SRCROOT)/../build/release
CFLAGS  += -DDEBUG -g
BUILDROOT= $(SRCROOT)/../build/debug
LIBS     = -Wl,--whole-archive $(BUILDROOT)/lib-rogue-pc-core.a $(BUILDROOT)/lib-my-curses.a -Wl,--no-whole-archive

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)