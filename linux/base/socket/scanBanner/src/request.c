#include "request.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

int open_clientfd(const char *hostname, unsigned int port, int time) {
    int clientfd, retval;
    struct hostent *hp;
    struct sockaddr_in serveraddr;
    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    fd_set rset;

    FD_ZERO(&rset);

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        return -1;

    if ((hp = gethostbyname(hostname)) == NULL)
        return -2;

    //if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
    //    return -1;
    //}

    //if (setsockopt(clientfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
    //    return -1;
    //}

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char*) hp->h_addr,
           (char *) &serveraddr.sin_addr.s_addr, sizeof(hp->h_length));
    //serveraddr.sin_addr.s_addr = inet_addr(hostname);
    serveraddr.sin_port = htons(port);

    int val; //file discriptor

    val = fcntl(clientfd, F_GETFL, 0);
#ifdef DEBUG
    //fprintf(stdout, "Orig file discriptor: %d\n", val);
#endif

    fcntl(clientfd, F_SETFL, O_NONBLOCK); //设置为非阻塞

    val = fcntl(clientfd, F_GETFL, 0);

    if (connect (clientfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        FD_SET(clientfd, &rset);
        //set connect timeout
        retval = select(clientfd + 1, NULL, &rset, NULL, &timeout);
        //printf("%d %d\n", retval, clientfd);
        if (retval == -1) {
            //fprintf(stderr, "Select fail\n");
            return -1;
        } else if (retval == 0) {
            //fprintf(stderr, "Timeout\n");
            //fcntl(clientfd, F_SETFL,  val & ~O_NONBLOCK);
            return -1;
        } else if (retval > 0) {
            //设为阻塞
            fcntl(clientfd, F_SETFL,  val & ~O_NONBLOCK);
            return clientfd;
        }
    }

    return -1;

}
