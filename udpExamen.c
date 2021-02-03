//Hecho por Daniel Valverde Menasalvas para el examen de ASOR Febrero 2021

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s host port\n", argv[0]);
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

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */

        for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    /* Read datagrams and echo them back to sender */

    for (;;) {
        peer_addr_len = sizeof(struct sockaddr_storage);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1)
            continue;               /* Ignore failed request */

        char host[NI_MAXHOST], service[NI_MAXSERV];

        s = getnameinfo((struct sockaddr *) &peer_addr,
                        peer_addr_len, host, NI_MAXHOST,
                        service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0)
            printf("Received %ld bytes from %s:%s\n",
                    (long) nread, host, service);
        else
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        char response[50];
        struct stat fstat;
        buf[strlen(buf)-1] = '\0';
        if (stat(buf, &fstat) == -1) {
               perror("stat");
               strcpy(response, "There is no such file in this host.\n");
        }else {
            
            strcpy(response, "File type:                ");
            switch (fstat.st_mode & S_IFMT) {
            case S_IFBLK:  strcat(response,"block device\n");            break;
            case S_IFCHR:  strcat(response,"character device\n");        break;
            case S_IFDIR:  strcat(response,"directory\n");               break;
            case S_IFIFO:  strcat(response,"FIFO/pipe\n");               break;
            case S_IFLNK:  strcat(response,"symlink\n");                 break;
            case S_IFREG:  strcat(response,"regular file\n");            break;
            case S_IFSOCK: strcat(response,"socket\n");                  break;
            default:       strcat(response,"unknown?\n");                break;
            }
        }


        if (sendto(sfd, response, sizeof(response), 0,
                    (struct sockaddr *) &peer_addr,
                    peer_addr_len) != sizeof(response))
            fprintf(stderr, "Error sending response\n");
    }
    return 0;
}