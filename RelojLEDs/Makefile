CC = gcc
CFLAGS = -std=gnu99 $(SYS_CFLAGS) -Wall -g -O2 $(DEBUG)
LDLIBS = $(SYS_LDLIBS) -lwiringPi

# debug options
DEBUG='-DDEBUG(x)=x' -include stdio.h

# native RaspberryPi
SYS_CFLAGS = $(shell xeno-config --skin posix --cflags)
SYS_LDLIBS = $(shell xeno-config --skin posix --ldflags)

all: main

main:	main.o tasks.o screen.o fsm.o

clean:
	$(RM) *.o *~ main
