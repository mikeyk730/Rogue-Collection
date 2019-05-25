CC       = g++
SHELL    = /bin/sh
SRCROOT  = ../..
BUILDROOT= $(SRCROOT)/../build/

INCLUDES = -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses -I$(SRCROOT)/RogueVersions/Rogue_PC_Core
CFLAGS   = -Wall -Werror -shared -fPIC
LIBS     = -Wl,--whole-archive $(BUILDROOT)/lib-rogue-pc-core.a $(BUILDROOT)/lib-my-curses.a -Wl,--no-whole-archive

TARGET   = lib-rogue-pc-1-48.so
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)