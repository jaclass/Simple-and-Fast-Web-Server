CC = gcc
CFLAGS = -O2 -Wall -I ..
DYNAMIC = -shared -fpic

LIB = -lpthread

all: clean adder multiplier

adder: adder.c
	$(CC) $(CFLAGS) $(DYNAMIC) -o adder.so adder.c 

multiplier: multiplier.c
	$(CC) $(CFLAGS) $(DYNAMIC) -o multiplier.so multiplier.c 

clean:
	rm -f adder *~
