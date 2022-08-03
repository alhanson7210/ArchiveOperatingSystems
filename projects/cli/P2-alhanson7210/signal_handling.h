/**
 * @file signal_handling.h
 */
#ifndef _SIGNAL_HANDLING_H_
#define _SIGNAL_HANDLING_H_

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @param signo sig no
 */
void sigquithandler(int signo);

/**
 * @param signo sig no
 */
void sigusr1handler(int signo);

/**
 * @param signo sig no
 */
void siginthandler(int signo);

/**
 * @param void signals
 */
void create_signals();

#endif
