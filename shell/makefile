CC=gcc
CFLAGS=

.PHONY: clean

all:main.o
	gcc main.o -o main.a 

main.o: main.c
	gcc -c main.c -o main.o

clean:
	rm *.o *.a
