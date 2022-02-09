/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 */
#include <dlfcn.h>
#include <dirent.h>
#include "csapp.h"

int process_num = 16;

#define CGI_SCIPTS_PATH "./cgi-bin/"
#define MAX_CGI_BINS 1024
#define MAX_CGI_NAME_LEN 80

// Arrays of cgi scripts' names
static char cgi_bin_names[MAX_CGI_BINS][MAX_CGI_NAME_LEN];

// Arrays of cgi dynamic handler
static int (*cgi_funcs[MAX_CGI_BINS])(void);

// Number of cgi scripts
static int cgi_num = 0;


void initialize();
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
void create_process_pool(int num, int *process_ids, int listenfd);
void handle(int listenfd);
int get_dynamic_handler_index(char *name);


/* $begin servermain */
int main(int argc, char **argv)
{
    int listenfd, port;
    int process_id[process_num];

    /* Check command line args */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    initialize();
    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);
    create_process_pool(process_num, process_id, listenfd);

    for (int i = 0; i < process_num; i++) {
        int pid = Wait(NULL);
    }
    printf("server close!\n");
    close(listenfd);
}
/* $end servermain */

/*
 * initialize - initialize dynamic linking function
 */
/* $begin initialize */
void initialize()
{
    DIR *d;
    struct dirent *dir;
    d = opendir(CGI_SCIPTS_PATH);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strstr(dir->d_name, ".so"))
            {
                void *handle;
                char *error;
                char sopath[MAXLINE];

                strcat(sopath, CGI_SCIPTS_PATH);
                strcat(sopath, dir->d_name);

                /* Dynamically load the shared library that contains addvec() */
                handle = dlopen(sopath, RTLD_NOW);
                if (!handle)
                {
                    fprintf(stderr, "%s\n", dlerror());
                    exit(1);
                }

                /* Get a pointer to the  function we just loaded */
                cgi_funcs[cgi_num] = dlsym(handle, "main");
                if ((error = dlerror()) != NULL)
                {
                    fprintf(stderr, "%s\n", error);
                    exit(1);
                }

                char *ptr = strchr(dir->d_name, '.');
                *ptr = '\0';
                strcpy(cgi_bin_names[cgi_num], dir->d_name);

                memset(sopath,0,strlen(sopath));
                cgi_num++;
            }
        }
        closedir(d);
    }
}
/* $begin initialize */


/*
 * create_process_pool - create persistent processes that can handle requests
 */
/* $begin create_process_pool */
void create_process_pool(int num, int *process_ids, int listenfd)
{
    for (int i = 0; i < num; i++) {
        int process_id = Fork();
        if (process_id == 0) {  // worker process
            handle(listenfd);
            exit(0);
        } else {
            process_ids[i] = process_id;
        }
    }
}
/* $end create_process_pool */

/*
 * handle - persistently accept the connection and handle request
 */
/* $begin handle */
void handle(int listenfd)
{
    int connfd, clientlen;
    struct sockaddr_in clientaddr;
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
        doit(connfd);                                          
        Close(connfd);                                            
    }
}
/* $end handle */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);             // line:netp:doit:readrequest
    sscanf(buf, "%s %s %s", method, uri, version); // line:netp:doit:parserequest
    if (strcasecmp(method, "GET"))
    { // line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                       // line:netp:doit:endrequesterr
    read_requesthdrs(&rio); // line:netp:doit:readrequesthdrs

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs); // line:netp:doit:staticcheck
    if (stat(filename, &sbuf) < 0)
    { // line:netp:doit:beginnotfound
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    } // line:netp:doit:endnotfound

    if (is_static)
    { /* Serve static content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        { // line:netp:doit:readable
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size); // line:netp:doit:servestatic
    }
    else
    { /* Serve dynamic content */
        if (get_dynamic_handler_index(strrchr(filename, '/') + 1) >= cgi_num)
        { // line:netp:doit:executable
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny hasn't loaded the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs); // line:netp:doit:servedynamic
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    // printf("Handle by process: %d\n",  getpid());
    while (strcmp(buf, "\r\n"))
    { // line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        // printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * get_dynamic_handler_index - get the index of a cgi service
 */
/* $begin get_dynamic_handler_index */
int get_dynamic_handler_index(char * name)
{
    for (int i = 0; i < cgi_num; i++) {
        if (!strcmp(name, cgi_bin_names[i])) {
            return i;
        }
    }
    return cgi_num;
}
/* $end get_dynamic_handler_index */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin"))
    { /* Static content */                 // line:netp:parseuri:isstatic
        strcpy(cgiargs, "");               // line:netp:parseuri:clearcgi
        strcpy(filename, ".");             // line:netp:parseuri:beginconvert1
        strcat(filename, uri);             // line:netp:parseuri:endconvert1
        if (uri[strlen(uri) - 1] == '/')   // line:netp:parseuri:slashcheck
            strcat(filename, "home.html"); // line:netp:parseuri:appenddefault
        return 1;
    }
    else
    { /* Dynamic content */    // line:netp:parseuri:isdynamic
        ptr = index(uri, '?'); // line:netp:parseuri:beginextract
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, ""); // line:netp:parseuri:endextract
        strcpy(filename, ".");   // line:netp:parseuri:beginconvert2
        strcat(filename, uri);   // line:netp:parseuri:endconvert2
        return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);    // line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); // line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf)); // line:netp:servestatic:endserve

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);                        // line:netp:servestatic:open
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // line:netp:servestatic:mmap
    Close(srcfd);                                               // line:netp:servestatic:close
    Rio_writen(fd, srcp, filesize);                             // line:netp:servestatic:write
    Munmap(srcp, filesize);                                     // line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE];
    int index;

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    
    // get the index of dynamic handler
    index = get_dynamic_handler_index(strrchr(filename, '/') + 1);
    if (index >= cgi_num) {
        unix_error("cgi scripts not loaded");
    }

    // do not need to fork process here
    setenv("QUERY_STRING", cgiargs, 1);                         // line:netp:servedynamic:setenv
    int sfd = dup(STDOUT_FILENO);
    Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */    // line:netp:servedynamic:dup2
    cgi_funcs[index]();
    // Recover the stdout
    Dup2(sfd, STDOUT_FILENO);
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor="
                  "ffffff"
                  ">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */