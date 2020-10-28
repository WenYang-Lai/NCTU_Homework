CC = g++
INCLUDE_DIR = ./include
SRC_DIR = ./src
OBJ_DIR = ./obj
FLAGS =-I$(INCLUDE_DIR) -std=c++11 -g -O2 -Wunused-result

all: 
	-mkdir obj
	make server 

server: $(OBJ_DIR)/server.o 
	$(CC) $(FLAGS) $(OBJ_DIR)/server.o -o server

$(OBJ_DIR)/server.o: $(SRC_DIR)/server.cpp $(INCLUDE_DIR)/server.h 
	$(CC) $(FLAGS) -c $(SRC_DIR)/server.cpp -o $(OBJ_DIR)/server.o

clean:
	rm -rf $(OBJ_DIR)/
	rm -r server
