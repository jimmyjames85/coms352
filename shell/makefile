CC=gcc
CFLAGS=-Wall
# -ansi

.PHONY: clean git wait

all: List.o Job.o util.o main.o LList.o wait
	gcc ${CFLAGS} List.o Job.o util.o LList.o main.o -o wsh

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

wait: wait.c
	gcc wait.c -o waitn

clean:
	rm *.o *.a *.stackdump *.exe waitn wsh

