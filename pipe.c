#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    int pipefd[2];
    pid_t cpid;
    char buf;

    if (argc <= 2) {
    fprintf(stderr, "Usage: %s comand1 comand2 arg2_1 arg2_2 ...\n", argv[0]);
    exit(EXIT_FAILURE);
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {    /* Child reads from pipe */
        close(pipefd[1]);          /* Close unused write end */
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        execvp(argv[2],argv+2);
        perror("execvp");
        exit(EXIT_FAILURE);
        
    } else {            /* Parent writes argv[1] to pipe */
        close(pipefd[0]);          /* Close unused read end */
        dup2(pipefd[1],1);
        close(pipefd[1]);
        execlp(argv[1], argv[1], NULL);
        exit(EXIT_SUCCESS);
    }
}