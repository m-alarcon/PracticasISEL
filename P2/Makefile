CC=gcc
CFLAGS=-g -Wall -O -I. -DNDEBUG
LDFLAGS=
LDLIBS=-lwiringPi -lrt

all: main

main: main.o fsm.o

clean:
	$(RM) *.o *~ main
