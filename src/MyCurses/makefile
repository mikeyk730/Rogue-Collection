CC       = g++
SHELL    = /bin/sh
SRCROOT  = ..
BUILDROOT= $(SRCROOT)/../build/release

INCLUDES = -I$(SRCROOT)/Shared
CFLAGS   = -Wall -Werror -pedantic -fPIC

TARGET   = lib-my-curses.a
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

CFLAGS += -DDEBUG -g
BUILDROOT = $(SRCROOT)/../build/debug

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rvs -g $(BUILDROOT)/$(TARGET) $(OBJECTS)
#	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)