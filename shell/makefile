CC=gcc
CFLAGS=
# -ansi

.PHONY: clean git

all:main.o
	gcc ${CFLAGS} main.o -o main.a 

main.o: main.c
	gcc ${CFLAGS} -c main.c -o main.o

clean:
	rm *.o *.a

