#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#define BUF_SIZE 80

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s server port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }


    sfd = socket(result->ai_family, result->ai_socktype,
            result->ai_protocol);
    if (sfd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(sfd, result->ai_addr, result->ai_addrlen) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    /* Read datagrams and echo them back to sender */

    for (;;) {
        fd_set rfds;
        int retval;

        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(sfd, &rfds);

        retval = select(sfd+1, &rfds, NULL, NULL, NULL);

        if (retval == -1)
            perror("select()");

        char outstr[200];
        time_t t;
        struct tm *tmp;

        t = time(NULL);
        tmp = localtime(&t);
        if (tmp == NULL) {
            perror("localtime");
            exit(EXIT_FAILURE);
        }
        size_t tSize = strftime(outstr, sizeof(outstr), "%H:%M:%S", tmp);
        if (tSize == 0) {
            fprintf(stderr, "strftime returned 0");
            exit(EXIT_FAILURE);
        }
        outstr[tSize] = '\n';

        if (FD_ISSET(sfd, &rfds)){
            peer_addr_len = sizeof(struct sockaddr_storage);
            nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                    (struct sockaddr *) &peer_addr, &peer_addr_len);
            if (nread == -1){
                perror("recvfrom");               /* Ignore failed request */
                exit(EXIT_FAILURE);
            }
            
            char host[NI_MAXHOST], service[NI_MAXSERV];

            s = getnameinfo((struct sockaddr *) &peer_addr,
                            peer_addr_len, host, NI_MAXHOST,
                            service, NI_MAXSERV, NI_NUMERICSERV);
            if (s == 0)
                printf("Received %ld bytes from %s:%s\n",
                        (long) nread, host, service);
            else
                fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));


            if (sendto(sfd, outstr, tSize+1, 0,
                        (struct sockaddr *) &peer_addr,
                        peer_addr_len) != tSize+1)
                fprintf(stderr, "Error sending response through red\n");
        }
        else
        {
            size_t s = read(0, buf, BUF_SIZE);
            if (s == -1){
                perror("read");
                exit(EXIT_FAILURE);
            }
            printf("Received %ld bytes from console\n", (long) s);
            if (write(0, outstr, tSize+1) != tSize+1)
                fprintf(stderr, "Error sending response through console\n");
        }
    }
}