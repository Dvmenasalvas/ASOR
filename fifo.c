#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char ** argv){
    char* buf[80];
    int tb1 = open("./tuberia1", O_RDONLY|O_NONBLOCK);
    int tb2 = open("./tuberia2", O_RDONLY|O_NONBLOCK);
    if (tb1 == -1 || tb2 == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    while(1){
        fd_set rfds;
        int retval;

        FD_ZERO(&rfds);
        FD_SET(tb1, &rfds);
        FD_SET(tb2, &rfds);

        int maxfd = tb1 > tb2 ? tb1 : tb2;
        retval = select(maxfd+1, &rfds, NULL, NULL, NULL);
        /* Don't rely on the value of tv now! */
        

        if (retval == -1)
            perror("select()");
        else if (FD_ISSET(tb1, &rfds)){
            size_t s = read(tb1, buf, sizeof(buf));
            if (s == -1){
                perror("read");
                exit(EXIT_FAILURE);
            } else if (s == 0){
                close(tb1);
                tb1 = open("./tuberia1", O_RDONLY|O_NONBLOCK);
            } else {
                printf("Leido desde tubería 1: %s \n", buf);
            }
        }
        if (FD_ISSET(tb2, &rfds)){
            size_t s = read(tb2, buf, sizeof(buf));
            if (s == -1){
                perror("read");
                exit(EXIT_FAILURE);
            } else if (s == 0){
                close(tb2);
                tb2 = open("./tuberia2", O_RDONLY|O_NONBLOCK);
            } else {
                printf("Leido desde tubería 2: %s \n", buf);
            }
        }
    }


    close(tb1);close(tb2);

    return 0;
}