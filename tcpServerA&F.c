#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>

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
    int clisd;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s host port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
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

    sfd = socket(result->ai_family, result->ai_socktype,result->ai_protocol);
    if (sfd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (bind(sfd, result->ai_addr, result->ai_addrlen) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }  

    freeaddrinfo(result);           /* No longer needed */
    if(listen(sfd, 15) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int hijos = 0;
    /* Read datagrams and echo them back to sender */
    for (;;) {
        clisd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
        
        pid_t pid = fork();
        if (pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0){
            char host[NI_MAXHOST], service[NI_MAXSERV];
            if (getnameinfo((struct sockaddr *) &peer_addr,
                peer_addr_len, host, NI_MAXHOST,
                service, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) == -1){
                    perror("getnameinfo");
                    exit(EXIT_FAILURE);
                }
            printf("Conexión establecida: Server %s Host %s PID %i\n", host, service, getpid());
            while(nread = recv(clisd, buf, BUF_SIZE, 0)){
                buf[nread] = '\0';
                printf("Mensaje: %s", buf);
                send(clisd, buf, nread, 0);
            }
            printf("Conexión terminada con host %s port %s\n", host,service);
        }
        else {
            int sta;
            waitpid(pid, &sta, WNOHANG);
            printf("No more zombies\n");
        }
        close(clisd);
    }
    return 0;
}