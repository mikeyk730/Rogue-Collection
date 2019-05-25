CC       = g++
SHELL    = /bin/sh
SRCROOT  = ..

INCLUDES = -I$(SRCROOT)/Shared
CFLAGS   = -Wall -Werror

TARGET   = lib-my-curses.a
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