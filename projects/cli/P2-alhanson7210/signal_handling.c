/**
 * @file signal_handling.h
 */
#include "signal_handling.h"
#include "prompt.h"
#include "debug.h"

#define SIGNAL_SIZE 3
#define BUFFER 64

/**
 * @param signo signal number
 */
void sigquithandler(int signo)
{
	printf("\n");
	fflush(stdout);
	LOG("signal exit code %d\n", signo);
	exit(signo);
}

/**
 * @param signo signal number
 */
void sigusr1handler(int signo)
{
	printf("\n");
	fflush(stdout);
	LOG("signal exit code %d\n", signo);
	exit(signo);
	
}

/**
 * @param signo signal number
 */
void siginthandler(int signo)
{
	if(isatty(STDIN_FILENO)) 
	{
		printf("\n");
		fflush(stdout);
		LOG("signal exit code %d\n", signo);
		setExitCode(true);
		//print_shell_prompt("flash", 0);
	}
}

/**
 * @param void create signals
 */
void create_signals()
{
	signal(SIGQUIT, sigquithandler);
	signal(SIGUSR1, sigusr1handler);
	signal(SIGINT, siginthandler);
}
