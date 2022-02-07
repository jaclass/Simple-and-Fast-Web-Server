#include "cgi.h"
#include "csapp.h"

char pipename[MAXLINE];
// int cgi_server_read_fd;
// int cgi_server_write_fd;

void cgi_request(char *pipename, char *request) {
    // Open FIFO and write the request
    int fd = Open(pipename, O_WRONLY, 0);
    Rio_writen(fd, request, strlen(request) + 1);
    Close(fd);
}

void cgi_response(char *pipename, char *buf) {
    rio_t rio;
    int fd = Open(pipename, O_RDONLY, 0);
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("resp: %s\n", buf);
    Close(fd);
}

void cgi_pipename(char *filename, char *pipename) {
    char *ptr;
    strcpy(pipename, FIFO_PREFIX);
    ptr = strrchr(filename, '/');
    strcat(pipename, ptr + 1);
    strcat(pipename, FIFO_SUFFIX);
}

void cgi_prepare(char **argv, int id) {
    // create fifo pipe
    cgi_pipename(argv[0], pipename);
    mkfifo(pipename, PIPE_MOD);

    // open read & write fd
    // cgi_server_read_fd = open(pipename, O_RDONLY);
    // cgi_server_write_fd = open(pipename, O_WRONLY);
}

int cgi_read(char *buf, int max_line) {
    int cgi_server_read_fd = open(pipename, O_RDONLY);
    read(cgi_server_read_fd, buf, max_line);
    close(cgi_server_read_fd);
    return 1;
}

void cgi_write(char *buf) {
    int cgi_server_write_fd = open(pipename, O_WRONLY);
    write(cgi_server_write_fd, buf, strlen(buf));
    close(cgi_server_write_fd);
}
