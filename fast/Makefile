CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

server: clean server.c csapp.o cgi.o
	$(CC) $(CFLAGS) -o server server.c csapp.o cgi.o $(LIB)

pipe_demo: pipe_read pipe_write

cgi_test:
	$(CC) $(CFLAGS) -o cgi cgi.c

pipe_read: clean pipe_read.c csapp.o cgi.o
	$(CC) $(CFLAGS) -o adder pipe_read.c csapp.o cgi.o $(LIB)

pipe_write:
	$(CC) $(CFLAGS) -o pipe_write pipe_write.c 

code_cache: code_cache.c csapp.o
	$(CC) $(CFLAGS) -o code_cache code_cache.c csapp.o $(LIB)

cgi:
	(cd cgi-bin; make)

csapp.o:
	$(CC) $(CFLAGS) -c csapp.c

cgi.o:
	$(CC) $(CFLAGS) -c cgi.c

clean:
	rm -f *.o pipe_read pipe_write code_cache *~