CC=gcc
CFLAGS=-ansi -Wall -O3

TARGET=zbrainfuck zbfc

all: $(TARGET)
clean:
	rm -f *~ $(TARGET)
