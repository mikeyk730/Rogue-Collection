CC       = g++
SHELL    = /bin/sh
SRCROOT  = ../..
BUILDROOT= $(SRCROOT)/../build/

INCLUDES = -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses -I$(SRCROOT)/RogueVersions/Rogue_PC_Core
CFLAGS   = -Wall -Werror -shared -fPIC

TARGET   = lib-rogue-pc-1-48.so
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)