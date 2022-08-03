/**
 * @file redirection.h
 */
#ifndef _REDIRECTION_H_
#define _REDIRECTION_H_

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief command line struct
 */
struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
};

/**
 * @param cmds command line array for pipeline/redirection
 */
void execute_pipeline(struct command_line * cmds);

#endif
