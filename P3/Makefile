CC=gcc
CFLAGS=-g -Wall -O -I. -DNMEDIDATIEMPO $(shell xeno-config --skin posix --cflags)
LDFLAGS=
LDLIBS=-lwiringPi $(shell xeno-config --skin posix --ldflags)

all: main

main: main.o fsm.o

clean:
	$(RM) *.o *~ main
