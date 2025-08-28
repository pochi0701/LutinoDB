CC=g++
CFLAGS=-c -g -pg -pthread -Wall -D_DEBUG
#CFLAGS=-c -O1 -Wall
LDFLAGS=-g -pg -pthread
#LDFLAGS= 

SOURCES=  \
msql.cpp \
wizd_String.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all: msql 

msql: msql.o $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f msql msql.o $(OBJECTS)
