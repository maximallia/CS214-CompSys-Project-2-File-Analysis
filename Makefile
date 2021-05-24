
CC = gcc
CFLAGS = -Wall
PFLAGS = -pthread
MFLAGS = -lm

COMP= CC PFLAGS -o detector Asst2.c

all: detector

detector: Asst2.c
	gcc -pthread -Wall -o detector Asst2.c -lm

clean:
	rm -f detector *.o
