CC = gcc
CFLAGS = -O2 -Wall -I ..

LIB = -lpthread

all: adder

adder: adder.c
	$(CC) $(CFLAGS) -o adder adder.c ../cgi.o ../csapp.o $(LIB)

clean:
	rm -f adder *~
