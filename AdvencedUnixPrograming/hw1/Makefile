CXX = g++
INCLUDE_DIR = ./include
SRC_DIR = ./src
OBJ_DIR = ./obj
CFLAGS = -std=c++11 -g -O2 -Wunused-result

PROG = main

all: $(PROG)

%: $(SRC_DIR)/%.cpp
	$(CXX) -o hw1 $(CFLAGS) $<

clean:
	rm -f $(PROG)
