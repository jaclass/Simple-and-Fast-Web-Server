#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include "csapp.h"
			
int main(int argc, char**argv)
{
	// create anonymous and executable memory file descriptor
  	int fd = syscall(SYS_memfd_create, "cached_code", 0);
    if (fd == -1) {
		err(1, "%s failed", "memfd_create");
	}

	char filename[] = "./cgi-bin/adder";
	int srcfd;
    char *srcp;
	struct stat sbuf;
	char *emptylist[] = {NULL};

	// read executable binary into memory
	if (stat(filename, &sbuf) < 0)
    { //line:netp:doit:beginnotfound
        err(1, "%s failed", filename);
        return -1;
    }

	srcfd = Open(filename, O_RDONLY, 0);                        //line:netp:servestatic:open
    srcp = Mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0); //line:netp:servestatic:mmap
    Close(srcfd);                                               //line:netp:servestatic:close
    Rio_writen(fd, srcp, sbuf.st_size);
	Munmap(srcp, sbuf.st_size);

	setenv("QUERY_STRING", "1&3", 1);
	fexecve(fd, emptylist, environ);
}