CC = g++
INCLUDE_DIR = ./include
SRC_DIR = ./src
OBJ_DIR = ./obj
FLAGS =-I$(INCLUDE_DIR) -std=c++11 -g -O2 -Wunused-result

all: 
	-mkdir obj
	make server 

server: $(OBJ_DIR)/sh.o $(OBJ_DIR)/server.o $(OBJ_DIR)/function.o
	$(CC) $(FLAGS) $(OBJ_DIR)/sh.o $(OBJ_DIR)/server.o $(OBJ_DIR)/function.o -o server

$(OBJ_DIR)/sh.o: $(SRC_DIR)/sh.cpp $(INCLUDE_DIR)/sh.h
	$(CC) $(FLAGS) -c $(SRC_DIR)/sh.cpp -o $(OBJ_DIR)/sh.o

$(OBJ_DIR)/server.o: $(SRC_DIR)/server.cpp $(INCLUDE_DIR)/server.h 
	$(CC) $(FLAGS) -c $(SRC_DIR)/server.cpp -o $(OBJ_DIR)/server.o

$(OBJ_DIR)/function.o: $(SRC_DIR)/function.cpp $(INCLUDE_DIR)/function.h
	$(CC) $(FLAGS) -c $(SRC_DIR)/function.cpp -o $(OBJ_DIR)/function.o
clean:
	rm -rf $(OBJ_DIR)/
	rm -r server
