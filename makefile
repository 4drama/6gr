CC=gcc

CFLAGS=-c -DSFML_STATIC -std=c++14 -g -pthread -IC:\libraries\SFML-2.4.2_gcc64\include
LIBFLAGS=-LC:/libraries/SFML-2.4.2_gcc64/lib
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lstdc++

all:
	$(CC) $(CFLAGS) main.cpp -o main.o
	$(CC) $(CFLAGS) map.cpp -o map.o
	$(CC) $(CFLAGS) control.cpp -o control.o
	$(CC) $(LIBFLAGS) main.o map.o control.o -o main.exe $(LDFLAGS)

clean:
	rm -rf main.o map.o control.o main.exe
