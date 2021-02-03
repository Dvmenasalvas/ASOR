#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

volatile int sigint = 0, sigtstp = 0;

static void
handler(int sig, siginfo_t *si, void *unused)
{
    if (sig == SIGINT){
        printf("SIGINT recibida, llevamos %i", sigint);
        sigint++;
    }
    if (sig == SIGTSTP){
        printf("SIGTSTP recibida, llevamos %i", sigtstp);
        sigtstp++;
    }
}

int
main(int argc, char *argv[])
{
    struct sigaction sa;
    sigset_t set;
    sigemptyset(&set);

    if (sigaction(SIGINT, NULL, &sa) == -1)
        perror("sigaction");
    sa.sa_handler = handler;
     if (sigaction(SIGINT, &sa, NULL) == -1)
        perror("sigaction");
    if (sigaction(SIGTSTP, NULL, &sa) == -1)
        perror("sigaction");
    sa.sa_handler = handler;
     if (sigaction(SIGTSTP, &sa, NULL) == -1)
        perror("sigaction");

    int count = 0;
    while (count < 10) {
        sigsuspend(&set);
    }
    printf("SIGINT captured: %i\n", sigint);
	printf("SIGTSTP captured: %i\n", sigtstp);

    
    exit(EXIT_SUCCESS);
}