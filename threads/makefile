CC=gcc
CFLAGS= -pthread -Wall
OBJECTS= main.o

.PHONY: clean all

all: $(OBJECTS)
	gcc ${CFLAGS}  $(OBJECTS) -o encrypt

%.o: %.c
	gcc -c  -o $@ $< ${CFLAGS} 

clean:
	rm -f *.o *.a *.exe encrypt

