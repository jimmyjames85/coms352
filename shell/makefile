CC=gcc
CFLAGS=-Wall
# -ansi

.PHONY: clean git

all: List.o Job.o main.o 
	gcc ${CFLAGS} List.o Job.o main.o -o main.a 

main.o: main.c 
	gcc ${CFLAGS} -c main.c -o main.o

List.o: List.c
	gcc ${CFLAGS} -c List.c -o List.o

Job.o: Job.c
	gcc ${CFLAGS} -c Job.c -o Job.o
clean:
	rm *.o *.a

