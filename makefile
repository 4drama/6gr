CC=gcc

CFLAGS=-c -DSFML_STATIC -std=c++14 -g -pthread -IC:\libraries\SFML-2.4.2_gcc64\include
LIBFLAGS=-LC:/libraries/SFML-2.4.2_gcc64/lib
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lstdc++

SRC_DIR= ./src/
OBJ_DIR= ./obj/
BIN_DIR= ./bin/

all: map client control gui main
	$(CC) $(LIBFLAGS) $(OBJ_DIR)main.o $(OBJ_DIR)map.o $(OBJ_DIR)control.o \
	$(OBJ_DIR)gui.o $(OBJ_DIR)client.o -o $(BIN_DIR)game.exe $(LDFLAGS)

map:
	$(CC) $(CFLAGS) $(SRC_DIR)map.cpp -o $(OBJ_DIR)map.o

client:
	$(CC) $(CFLAGS) $(SRC_DIR)client.cpp -o $(OBJ_DIR)client.o

control:
	$(CC) $(CFLAGS) $(SRC_DIR)control.cpp -o $(OBJ_DIR)control.o

gui:
	$(CC) $(CFLAGS) $(SRC_DIR)gui.cpp -o $(OBJ_DIR)gui.o

main:
	$(CC) $(CFLAGS) $(SRC_DIR)main.cpp -o $(OBJ_DIR)main.o

play:
	run.bat

clean:
	rm -rf  $(OBJ_DIR)*.o $(BIN_DIR)*.exe
