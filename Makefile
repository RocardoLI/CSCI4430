CC=gcc
CFLAGS=-Wall

EXE=server client
all: ${EXE}

clean:
	rm -f ${EXE}
