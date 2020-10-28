GXX = g++
CFLAGS = -I ./include/ -g -Wall -std=c++11
SRC_DIR = ./src

PROG = shell

all: $(PROG)

jobs.o: $(SRC_DIR)/jobs.cpp
	$(CXX) -o $@ $(CFLAGS) -c $^

shell.o: $(SRC_DIR)/shell.cpp
	$(CXX) -o $@ $(CFLAGS) -c $^

shell: shell.o jobs.o
	$(CXX) -o $@ $(CFLAGS) $^

clean:
	rm -f jobs.o shell.o shell
