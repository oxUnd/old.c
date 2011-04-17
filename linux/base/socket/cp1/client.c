/**
 * socket client
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>

#define ERROR_NO -1
#define EQUAL(a, b)  (a) == (b) ? 1 : 0
#define TEST(a, b, c) \
    if (EQUAL((a), (b))) { \
        printf("[%5d] function %s(...) return value on error.\n", \
            (__LINE__),\
            (c)); \
        return -1; }

static void show_help(void);

int main(int argc, char *argv[])
{
    if (argc != 5) {
        show_help();
        return -1;
    }
    int o;
    int port;
    char *hostname;
    while (-1 != (o = getopt(argc, argv, "h:p:"))) {
        switch (o) {
        case 'h':
            hostname = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        default:
            show_help();
            return -1;
        }
    }

    int clientfd;
    int con;
    int n;
    struct hostent *hp;
    struct sockaddr_in server_addr;
    char buf[1024] = "";
    TEST((clientfd = socket(AF_INET, SOCK_STREAM, 0)), ERROR_NO, "socket");
    TEST((hp = gethostbyname(hostname)), NULL, "gethostbyname");
    bzero((char *) & server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *) hp->h_addr,
            (char *) &server_addr.sin_addr.s_addr, hp->h_length);
    server_addr.sin_port = htons(port);
    TEST((con = connect(clientfd, (struct sockaddr *) &server_addr, sizeof(server_addr))), ERROR_NO, "connect");

    while (fgets(buf, 1024, stdin) != NULL) {
        n = write(clientfd, buf, sizeof(buf));
        read(clientfd, buf, n); 
        fputs(buf, stdout);
    }
    close(clientfd);

    return 0;
}

static void show_help(void) 
{
    char *m = ""\
"-h <host>\n"\
"-p <port>\n"\
"\n"\
"Usage: ./client -h <host> -p <port>\n";
    printf("%s", m);
}
