CC=g++

CFLAGS=-ggdb3

all: ringmaster player

ringmaster: ringmaster.cpp potato.h
	$(CC) $(CFLAGS) -o $@ ringmaster.cpp

player: player.cpp potato.h
	$(CC) $(CFLAGS) -o $@ player.cpp

.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player