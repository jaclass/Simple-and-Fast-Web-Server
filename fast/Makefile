CC = gcc
CFLAGS = -g -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: server

server: clean server.c csapp.o cgi
	$(CC) $(CFLAGS) -rdynamic -o server server.c csapp.o $(LIB) -ldl

csapp.o:
	$(CC) $(CFLAGS) -c csapp.c

cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o tiny dll *~
	(cd cgi-bin; make clean)