CC       = g++
SHELL    = /bin/sh
SRCROOT  = ..
LIBROOT  = ../../lib

INCLUDES = -I/usr/include/SDL2 -I$(SRCROOT)/Shared -I$(SRCROOT)/Shared/Frontend -I$(SRCROOT)/MyCurses -I$(LIBROOT)/SDL2_ttf-2.0.14/include -I$(LIBROOT)/SDL2_image-2.0.1/include -I$(LIBROOT)/NativeFileDialog/src/include
LIBS     = -Wl,--whole-archive $(BUILDROOT)/lib-shared-frontend.a -Wl,--no-whole-archive -ldl -lSDL2 -lSDL2_ttf -lSDL2_image
CFLAGS   = -Wall -Werror -pedantic -pthread

TARGET   = rogue-collection-sdl.exe
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

BUILDROOT= $(SRCROOT)/../build/release
#CFLAGS  += -DDEBUG -g
#BUILDROOT= $(SRCROOT)/../build/debug

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CC) -g -c -o $@ $< $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)