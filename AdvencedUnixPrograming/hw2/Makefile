CXX = g++
CC = gcc
INCLUDE_DIR = ./include
SRC_DIR = ./src
OBJ_DIR = ./obj
CFLAGS = -g -shared -fPIC 
LINK = -ldl

PROG = inject


all: $(PROG)

%: $(SRC_DIR)/%.c
	$(CC) -o hw2.so $(CFLAGS) $< $(LINK) 

clean:
	rm -f $(PROG)
