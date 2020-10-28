CC = g++
INCLUDE_DIR = ./include
SRC_DIR = ./src
OBJ_DIR = ./obj
FLAGS =-I$(INCLUDE_DIR) -std=c++11 -g -O2 -Wunused-result

all: 
	-mkdir obj
	make hw4.cgi 

hw4.cgi: $(SRC_DIR)/cgi.cpp $(INCLUDE_DIR)/cgi.h
	$(CC) $(FLAGS) $(SRC_DIR)/cgi.cpp -o hw4.cgi

clean:
	rm -rf $(OBJ_DIR)/
	rm -r hw4.cgi
