CC       = g++
SHELL    = /bin/sh
SRCROOT  = ..
LIBROOT  = ../../lib

INCLUDES = -I/usr/include/SDL2 -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses -I$(LIBROOT)/SDL2_ttf-2.0.14/include -I$(LIBROOT)/SDL2_image-2.0.1/include -I$(LIBROOT)/NativeFileDialog/src/include
CFLAGS   = -Wall -Werror -shared -fPIC

TARGET   = rogue-collection-sdl.exe
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(TARGET)