CC       = g++
SHELL    = /bin/sh
SRCROOT  = ..
BUILDROOT= $(SRCROOT)/../build/

INCLUDES = -I$(SRCROOT)/Shared
CFLAGS   = -Wall -Werror -fPIC

TARGET   = lib-my-curses.a
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rvs $(BUILDROOT)/$(TARGET) $(OBJECTS)
#	$(CC) $(CFLAGS) -o $(BUILDROOT)/$(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)