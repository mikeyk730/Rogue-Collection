CC       = g++
SHELL    = /bin/sh
SRCROOT  = ../..

INCLUDES = -I$(SRCROOT)/Shared -I$(SRCROOT)/MyCurses
DEFINES  = #-DROGOMATIC_PROTOCOL_DEBUGGING
CFLAGS   = -Wall -Werror -pedantic -fPIC

TARGET   = lib-shared-frontend.a
SOURCES  = $(shell echo *.cpp)
HEADERS  = $(shell echo *.h)
OBJECTS  = $(SOURCES:.cpp=.o)

BUILDROOT= $(SRCROOT)/../build/release
#CFLAGS  += -DDEBUG -g
#BUILDROOT= $(SRCROOT)/../build/debug

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rvs $(BUILDROOT)/$(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) $(DEFINES)

clean:
	rm -f $(OBJECTS) $(BUILDROOT)/$(TARGET)