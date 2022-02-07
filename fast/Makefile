CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

server: clean server.c csapp.o cgi.o
	$(CC) $(CFLAGS) -o server server.c csapp.o cgi.o $(LIB)

code_cache: code_cache.c csapp.o
	$(CC) $(CFLAGS) -o code_cache code_cache.c csapp.o $(LIB)

cgi-bin-compile:
	(cd cgi-bin; make clean; make)

csapp.o:
	$(CC) $(CFLAGS) -c csapp.c

cgi.o:
	$(CC) $(CFLAGS) -c cgi.c

clean:
	rm -f *.o pipe_read pipe_write code_cache *~
	(cd cgi-bin; make clean)