GXX = g++
CFLAGS = -g -Wall -std=c++11
SRC_DIR = ./src

PROG = shell

all: $(PROG)

%:$(SRC_DIR)/%.cpp
	$(CXX) -o $@ $(CFLAGS) $^

