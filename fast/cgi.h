#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_PREFIX "/tmp/fifo/"
#define FIFO_SUFFIX "-cgi-pipe"
#define	MAXLINE	 8192
#define PIPE_MOD 0666

void cgi_request(char *pipename, char *request);
void cgi_response(char *pipename, char *buf);
void cgi_pipename(char *filename, char *pipename);
void cgi_prepare(char **argv, int id);
int cgi_read(char *buf, int max_line);
void cgi_write(char *buf);