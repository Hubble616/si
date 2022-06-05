// https://stackoverflow.com/questions/11208299/how-to-make-an-http-get-request-in-c-without-libcurl/35680609#35680609
/*
Paul Crocker
Muitas Modificações
*/
// Definir sta linha com 1 ou com 0 se não quiser ver as linhas com debug info.
#define DEBUG 0

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "csapp.h"

enum CONSTEXPR
{
    MAX_REQUEST_LEN = 1024
};
typedef struct
{
    char *request;
    char *request_template;
    char *hostname;
    unsigned short server_port; // default port
    char *file;
    int id;
    int thread_num;
} cop_t;

char buffer[BUFSIZ];
sem_t mutex[1000];
void *client(void *arg)
{
    in_addr_t in_addr;
    int request_len;
    int socket_file_descriptor;
    ssize_t nbytes;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;
    struct protoent *protoent;

    cop_t *copiar = (cop_t *)arg;
    int id = copiar->id;
    unsigned short server_port = copiar->server_port;
    char *hostname = copiar->hostname;
    char *request = copiar->request;
    char *request_template = copiar->request_template;
    char *file = copiar->file;
    int thread_num = copiar->thread_num;

    while (1)
    {

        for (int i = 0; i < thread_num; i++)
        {
            if (i != id)
            {
                sem_wait(&mutex[i]);
            }
        }

        request_len = snprintf(request, MAX_REQUEST_LEN, request_template, file, hostname);
        if (request_len >= MAX_REQUEST_LEN)
        {
            fprintf(stderr, "request length large: %d\n", request_len);
            exit(EXIT_FAILURE);
        }

        /* Build the socket. */
        protoent = getprotobyname("tcp");
        if (protoent == NULL)
        {
            perror("getprotobyname");
            exit(EXIT_FAILURE);
        }
        printf("Thread %d: Iniciando...\n", id);

        // Open the socket fechar
        socket_file_descriptor = Socket(AF_INET, SOCK_STREAM, protoent->p_proto);
        if (socket_file_descriptor == -1)
        {
            perror("Socket");
            exit(EXIT_FAILURE);
        }
        /* Build the address.fechar */
        // 1 get the hostname address
        hostent = Gethostbyname(hostname);
        if (hostent == NULL)
        {
            fprintf(stderr, "Erro no host (\"%s\")\n", hostname);
            exit(EXIT_FAILURE);
        }

        in_addr = inet_addr(inet_ntoa(*(struct in_addr *)(hostent->h_addr_list)));
        if (in_addr == (in_addr_t)-1)
        {
            fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
            exit(EXIT_FAILURE);
        }
        sockaddr_in.sin_addr.s_addr = in_addr;
        sockaddr_in.sin_family = AF_INET;
        sockaddr_in.sin_port = htons(server_port);

        /* Ligar ao servidor */
        Connect(socket_file_descriptor, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in));

        /* Send HTTP request. */
        Rio_writen(socket_file_descriptor, request, request_len);

        /* Read the response. */
        if (DEBUG)
            fprintf(stderr, "debug: before first read\n");

        rio_t rio;
        char buf[MAXLINE];

        /* Leituras das linhas da resposta . Os cabecalhos - Headers */
        const int numeroDeHeaders = 5;
        Rio_readinitb(&rio, socket_file_descriptor);
        for (int k = 0; k < numeroDeHeaders; k++)
        {
            Rio_readlineb(&rio, buf, MAXLINE);

            // Envio das estatisticas para o canal de standard error
            if (strstr(buf, "Stat") != NULL)
                fprintf(stderr, "STATISTIC : %s", buf);
        }

        // Ler o resto da resposta - o corpo de resposta.
        // Vamos ler em blocos caso que seja uma resposta grande.
        while ((nbytes = Rio_readn(socket_file_descriptor, buffer, BUFSIZ)) > 0)
        {
            if (DEBUG)
                fprintf(stderr, "debug: after a block read\n");
            // commentar a lina seguinte se não quiser ver o output
            Rio_writen(STDOUT_FILENO, buffer, nbytes);
        }

        if (DEBUG)
            fprintf(stderr, "debug: after last read\n");

        Close(socket_file_descriptor);
        for (int i = 0; i < thread_num; i++)
        {
            if (i != id)
            {
                sem_post(&mutex[i]);
            }
        }
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{

    enum CONSTEXPR
    {
        MAX_REQUEST_LEN = 1024
    };
    char request[MAX_REQUEST_LEN];
    char request_template[] = "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n";

    char *hostname = "localhost";
    unsigned short server_port = 8080; // default port
    char *file;
    char *fileindex = "/home.html";
    /*char *filedynamic = "/cgi-bin/adder?150&100";
    char filestatic = "/godzilla.jpg";*/

    if (argc < 4)
    {
        fprintf(stderr, "usage: %s [host] [portnum] [threads] [filename1]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    hostname = argv[1];
    server_port = strtoul(argv[2], NULL, 10);
    if (argc <= 5)
        file = argv[4];
    else
        file = fileindex; // ou escolher outra filedynamic filestatic

    int threads = atoi(argv[3]);
    if (threads > 1000)
    {
        fprintf(stderr, "Numero de threads muito grande\n");
        exit(EXIT_FAILURE);
    }

    pthread_t tid[threads];
    cop_t *h = (cop_t *)malloc(sizeof(cop_t));
    h->request = request;
    h->request_template = request_template;
    h->hostname = hostname;
    h->server_port = server_port;
    h->file = file;
    h->thread_num = threads;
    for (int i = 0; i < threads; i++)
    {
        sem_init(&mutex[i], 0, 1);
    }

    for (int i = 0; i < threads; i++)
    {
        h->id = i;
        pthread_create(&tid[i], NULL, client, (void *)h);
    }

    for (int i = 0; i < threads; i++)
    {
        sem_destroy(&mutex[i]);
    }

    pthread_exit(NULL);

    // construção do pedido de http
}