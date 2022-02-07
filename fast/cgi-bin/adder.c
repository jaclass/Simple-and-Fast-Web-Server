/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "cgi.h"
#include <stdlib.h>

int main(int argc, char ** argv)
{
    char *p;
    cgi_prepare(argv, 0);

    char buf[80];
    char arg1[80], arg2[80], content[80];
    int n1=0, n2=0;
    while (cgi_read(buf, 80))
    {
        // Print the read string and close
        printf("CGI Request: %s\n", buf);

        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p+1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);

        sprintf(content, "The answer is: %d\n", n1 + n2);
        printf("CGI Response: %s\n", content);
        
        cgi_write(content);
    }
    return 0;
}
