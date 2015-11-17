CC=gcc
CFLAGS= -pthread -Wall
#-Wall -lpthread
# -ansi

OBJECTS= main.o

.PHONY: clean all

all: $(OBJECTS)
	gcc ${CFLAGS}  $(OBJECTS) -o main.a

%.o: %.c
	gcc -c  -o $@ $< ${CFLAGS} 

clean:
	rm *.o *.a *.stackdump *.exe

