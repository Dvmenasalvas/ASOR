#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Usage: %s command arg1 arg2 ...", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Hijo
        pid_t demonSes = setsid();
        if (chdir("/tmp") == -1){
            perror("chdir");
            exit(EXIT_FAILURE);
        }
        printf("Proceso hijo:\n");
        printf("Id de proceso: %i\n", getpid());
        printf("Id de proceso padre: %i\n", getppid());
        printf("Id de grupo de proceso: %i\n", getgid());
        printf("Id de sesión: %i\n", getsid(getpid()));
        execvp(argv[1], argv+1);
    } else{
        int status;
        wait(status);
    }
    return 0;
}