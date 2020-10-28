CXX	= g++
CC	= gcc
CFLAGS	= -Wall -g -I/usr/include/ncurses
LDFLAGS	= -lncurses -pthread
PROGS	= othello

all: $(PROGS)

othello: game.o othello.o function.o
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< $(LDFLAGS)

clean:
	rm -f *.o *~ $(PROGS)
