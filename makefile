CC=gcc

CFLAGS=-c -DSFML_STATIC -std=c++14 -g -pthread -IC:\libraries\SFML-2.4.2_gcc64\include
LIBFLAGS=-LC:/libraries/SFML-2.4.2_gcc64/lib
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lstdc++

SRC_DIR= ./src/
OBJ_DIR= ./obj/
BIN_DIR= ./bin/

all:
	$(CC) $(CFLAGS) $(SRC_DIR)main.cpp -o $(OBJ_DIR)main.o
	$(CC) $(CFLAGS) $(SRC_DIR)map.cpp -o $(OBJ_DIR)map.o
	$(CC) $(CFLAGS) $(SRC_DIR)control.cpp -o $(OBJ_DIR)control.o
	$(CC) $(CFLAGS) $(SRC_DIR)gui.cpp -o $(OBJ_DIR)gui.o

	$(CC) $(LIBFLAGS) $(OBJ_DIR)main.o $(OBJ_DIR)map.o $(OBJ_DIR)control.o \
	$(OBJ_DIR)gui.o -o $(BIN_DIR)game.exe $(LDFLAGS)

play:
	run.bat

clean:
	rm -rf  $(OBJ_DIR)*.o $(BIN_DIR)*.exe
