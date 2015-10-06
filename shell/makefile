CC=gcc
CFLAGS=-Wall
# -ansi

.PHONY: clean git

all: List.o Job.o util.o main.o LList.o
	gcc ${CFLAGS} List.o Job.o util.o LList.o main.o -o main.a 

main.o: main.c 
	gcc ${CFLAGS} -c main.c -o main.o

List.o: List.c
	gcc ${CFLAGS} -c List.c -o List.o

Job.o: Job.c
	gcc ${CFLAGS} -c Job.c -o Job.o

util.o: util.c
	gcc ${CFLAGS} -c util.c -o util.o

LList.o: LList.c
	gcc ${CFLAGS} -c LList.c -o LList.o

clean:
	rm *.o *.a

