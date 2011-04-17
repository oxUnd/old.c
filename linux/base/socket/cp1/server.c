/**
 * socket server
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#define MAXSIZE 1024
#define ERROR_NO -1
#define EQUAL(a, b)  (a) == (b) ? 1 : 0
#define TEST(a, b, c) \
    if (EQUAL((a), (b))) { \
        printf("function %s(...) return value on error.\n", \
            (c)); \
        return -1; }

static void show_help(void);
unsigned int io_read(int fd, char *buf, int count);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        show_help();
        return -1;
    }
    int o;
    int port;
    while (-1 != (o = getopt(argc, argv, "a:p:"))) {
        switch (o) {
        case 'p':
            port = atoi(optarg);
            break;
        default:
            show_help();
            return -1;
        }
    }

    int listenfd;
    int optval = 1;
    int client_len;
    int connfd;
    int n;
    char *haddrp;
    char buf[MAXSIZE];
    struct hostent *hp;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    //Server listen

    TEST((listenfd = socket(AF_INET, SOCK_STREAM, 0)), ERROR_NO, "socket");
    TEST(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)), ERROR_NO, "setsockopt");
    bzero((char *) & server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    TEST(bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)), ERROR_NO, "bind");
    TEST(listen(listenfd, 1024), ERROR_NO, "listen");

    while (1) {
        client_len = sizeof(client_addr);
        TEST((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)), ERROR_NO, "accept");
        TEST((hp = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr, sizeof(client_addr.sin_addr.s_addr), AF_INET)), NULL, "gethostbyaddr");
        haddrp = inet_ntoa(client_addr.sin_addr);
        printf("Server connected to %s (%s)\n", hp->h_name, haddrp);
        while ((n = io_read(connfd, buf, 1024)) != 0) {
            printf("Server received %d bytes\n", n);
            write(connfd, buf, n);
        }
        close(connfd);
    }

    return 0;
}

static void show_help(void) 
{
    char *m = ""\
"-p <port>\n"\
"\n"\
"Usage: ./server -p <port>\n";
    printf("%s", m);
}

unsigned int io_read(int fd, char *buf, int count)
{
    int n = read(fd, buf, count);
    if (n != 0) 
        return strlen(buf) - 1; //'\n'
    return 0; 
}
