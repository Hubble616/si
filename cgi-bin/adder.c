/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#define MAXLINE 8192
#define USE_SHA1
#include "tomcrypt_hash.h"

int **proof_of_work(char *str, int dificulty)
{
    
    unsigned char tmp[20];
    hash_state md;
        //simple example
    // atemmpt to find nonce such that hash(string || atoi(nonce))=20 bytes where the first n bytes are 0
    // returns nonce and hash
    int k=0;
    char *s2 = malloc(sizeof(char)*20);
    for (int i=0;i<dificulty;i++)
        s2[i]='\x00';
    s2[dificulty]='\0';

    while (1)
    {
        //generate nonce
        char nonce[10];
        sprintf(nonce, "%d", k);
        //generate hash
        sha1_init(&md);
        sha1_process(&md, (unsigned char *)str, (unsigned long)strlen(str));
        sha1_process(&md, (unsigned char *)nonce, (unsigned long)strlen(nonce));
        sha1_done(&md, tmp); 
        if (memcmp(tmp, s2, dificulty) == 0)
        {
            int **ret = malloc(2*sizeof(int*));
            ret[0] = malloc(sizeof(int));
            ret[1] = malloc(20*sizeof(int));
            *ret[0] = atoi(nonce);
            for (int i=0;i<20;i++)
                ret[1][i] = tmp[i];
            return ret;
        }
        k++;
    }



}

int main(void)
{
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1, n2;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL)
    {
        p = strchr(buf, '&');
        *p = '\0'; // anular o & dentro do string p -  para aproveitar no strcpy a seguir
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }
    int **result = malloc(2*sizeof(int*));
    result[0] = malloc(sizeof(int));
    result[1] = malloc(20*sizeof(int));
    result = proof_of_work(arg1, n1);


    /* Make the response body */
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
    sprintf(content, "%sThe nonce is: %d\r\n<p>", content, *result[0]);
    for (int i=0;i<20;i++)
        sprintf(content, "%s%x", content, result[1][i]);
    sprintf(content, "%s<p> \nThanks for visiting!\r\n", content);

    /* Generate the HTTP response */
    /* Paul Crocker - changed so that headers always produced on the parent process */
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");

    printf("%s", content);
    fflush(stdout);
    exit(0);
}
/* $end adder */
