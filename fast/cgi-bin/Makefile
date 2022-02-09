CC = gcc
CFLAGS = -O2 -Wall -I ..
DYNAMIC = -shared -fpic

LIB = -lpthread

all: clean adder multiplier adder-bin multiplier-bin

adder: adder.c
	$(CC) $(CFLAGS) $(DYNAMIC) -o adder.so adder.c 

multiplier: multiplier.c
	$(CC) $(CFLAGS) $(DYNAMIC) -o multiplier.so multiplier.c 

adder-bin: adder.c
	$(CC) $(CFLAGS) $(DYNAMIC) -o adder adder.c 

multiplier-bin: multiplier.c
	$(CC) $(CFLAGS) $(DYNAMIC) -o multiplier multiplier.c 

clean:
	rm -f adder *~
