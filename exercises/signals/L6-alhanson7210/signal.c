/**
 * signal.c
 *
 * Demonstrates signal handling. During normal operation this program counts up
 * each second, starting from 1. Upon receiving SIGINT, the final count will be
 * printed and the program will exit. Many programs use this type of signal
 * handling to clean up and gracefully exit when the user presses Ctrl+C.
 *
 * Compile: gcc -g -Wall signal.c -o signal
 * Run: ./signal
 */

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned long count = 0;
pid_t pid;

void sigalrm_handler(int signo) {
    count = 0;
    printf("\nSig alarm set count to: %ld\n", count);
}

void sigquit_handler(int signo) {
	printf("\nSig quit retrieved: count was %ld and is now ", count);
	count += 5;
	printf("%ld\n", count);
	fflush(stdout);
	//kill(pid, SIGQUIT);
	perror("sigquit_handle");
	exit(signo);
}

void sigusr1_handler(int signo) {
	printf("\nSig user 1 retrieved: count was %ld and is now ", count);
	count += 10;
	printf("%ld\n", count);
	exit(signo);
}

void sigint_handler(int signo) {
    printf("\n");
    printf("Final count is: %ld\n", count);
    printf("Goodbye!\n");
    exit(0);
}

int main(void) {
	pid = getpid();
    printf("Starting up. My PID = %d\n", pid);

    /* Set up our signal handler. SIGINT can be sent via Ctrl+C */
    signal(SIGALRM, sigalrm_handler);
	signal(SIGQUIT, sigquit_handler);
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGINT, sigint_handler);
    
    while (true) {
    	if(count == 0) alarm(15);
        count++;
        printf("Count: %ld\n", count);
        sleep(1);
    }

    return 0;

}
