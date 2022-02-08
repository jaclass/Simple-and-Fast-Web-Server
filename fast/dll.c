/* $begin dll */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include "csapp.h"

#define CGI_SCIPTS_PATH "./cgi-bin/"
#define MAX_CGI_BINS 1024
#define MAX_CGI_NAME_LEN 80

// Arrays of cgi scripts' names
static char cgi_bin_names[MAX_CGI_BINS][MAX_CGI_NAME_LEN];

// Arrays of cgi dynamic handler
static int (*cgi_funcs[MAX_CGI_BINS])(void);

// Number of cgi scripts
static int cgi_num = 0;

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

int main()
{   
    // prove setenv belongs to process
    if (fork() == 0)
    { /* child */ //line:netp:servedynamic:fork
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", "11&12", 1);
        exit(0);
    }
    wait(NULL);

    initialize();
    for (int i = 0; i < cgi_num; i++) {
        printf ("Filename: %s\n", cgi_bin_names[i]);
        cgi_funcs[i]();
    }

    
    return 0;
}
/* $end dll */