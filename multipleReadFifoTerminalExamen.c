//Hecho por Daniel Valverde Menasalvas para el examen de ASOR Febrero 2021

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    char buf[BUF_SIZE];
    
    //We open our fifo in read only mode
    int pfd = open("./tuberia", O_RDONLY|O_NONBLOCK);
    if (pfd == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    while(1){
        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(pfd, &rfds);

        /* Wait up to five seconds. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(1+pfd, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */
        
        if (retval == -1)
            perror("select()");
        else if (retval){
            if(FD_ISSET(0, &rfds)){
                read(0, buf, BUF_SIZE);
                printf("Data received from terminal: %s\n" , buf);
            } 
            else if (FD_ISSET(pfd, &rfds)){
                read(pfd, buf, BUF_SIZE);
                printf("Data received from fifo: %s\n" , buf);
                close(pfd);
                pfd = open("./tuberia", O_RDONLY|O_NONBLOCK);
            }
        }
        else{
            printf("No data within five seconds.\n");
            break;
        }
    }

    exit(EXIT_SUCCESS);
}