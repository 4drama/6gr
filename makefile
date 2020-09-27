CC=gcc

CFLAGS=-c -DSFML_STATIC -std=c++17 -g -pthread -IG:\libraries\SFML-2.5.1\include
LIBFLAGS=-LG:/libraries/SFML-2.5.1/lib
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lstdc++

SRC_DIR= ./src/
OBJ_DIR= ./obj/
BIN_DIR= ./bin/

all: $(OBJ_DIR)map.o $(OBJ_DIR)client.o $(OBJ_DIR)control.o \
	$(OBJ_DIR)gui.o $(OBJ_DIR)main.o $(OBJ_DIR)unit.o $(OBJ_DIR)items.o $(OBJ_DIR)window.o \
	$(OBJ_DIR)math.o $(OBJ_DIR)effects.o $(OBJ_DIR)garage.o $(OBJ_DIR)ai.o

	$(CC) $(LIBFLAGS) $(OBJ_DIR)main.o $(OBJ_DIR)map.o $(OBJ_DIR)control.o \
	$(OBJ_DIR)gui.o $(OBJ_DIR)client.o $(OBJ_DIR)unit.o $(OBJ_DIR)window.o \
	$(OBJ_DIR)items.o $(OBJ_DIR)math.o $(OBJ_DIR)effects.o $(OBJ_DIR)garage.o \
	$(OBJ_DIR)ai.o \
	-o $(BIN_DIR)game.exe $(LDFLAGS)

$(OBJ_DIR)unit.o: $(SRC_DIR)unit.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)unit.cpp -o $(OBJ_DIR)unit.o

$(OBJ_DIR)items.o: $(SRC_DIR)items.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)items.cpp -o $(OBJ_DIR)items.o

$(OBJ_DIR)map.o: $(SRC_DIR)map.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)map.cpp -o $(OBJ_DIR)map.o

$(OBJ_DIR)client.o: $(SRC_DIR)client.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)client.cpp -o $(OBJ_DIR)client.o

$(OBJ_DIR)control.o: $(SRC_DIR)control.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)control.cpp -o $(OBJ_DIR)control.o

$(OBJ_DIR)gui.o: $(SRC_DIR)gui.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)gui.cpp -o $(OBJ_DIR)gui.o

$(OBJ_DIR)window.o: $(SRC_DIR)window.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)window.cpp -o $(OBJ_DIR)window.o

$(OBJ_DIR)math.o: $(SRC_DIR)math.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)math.cpp -o $(OBJ_DIR)math.o

$(OBJ_DIR)effects.o: $(SRC_DIR)effects.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)effects.cpp -o $(OBJ_DIR)effects.o

$(OBJ_DIR)garage.o: $(SRC_DIR)garage.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)garage.cpp -o $(OBJ_DIR)garage.o

$(OBJ_DIR)ai.o: $(SRC_DIR)ai.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)ai.cpp -o $(OBJ_DIR)ai.o

$(OBJ_DIR)main.o: $(SRC_DIR)main.cpp
	$(CC) $(CFLAGS) $(SRC_DIR)main.cpp -o $(OBJ_DIR)main.o

play:
	run.bat

clean:
	rm -rf  $(OBJ_DIR)*.o $(BIN_DIR)*.exe
