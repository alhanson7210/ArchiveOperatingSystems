/**
 * @file prompt.h
 */
#ifndef _PROMPT_H_
#define _PROMPT_H_

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFERSIZE 128

/**
 * @param shellname name of shell
 * @param exitcode error code
 */
void print_shell_prompt(char * shellname, int exitcode);

/**
 * @param void increment
 */
void increment_cmd();
/**
 * @param void get global command number
 */
unsigned int get_cmd();

/**
 * @param newcode new error code
 */
void setExitCode(bool newcode);

/**
 * @param void is there a new exit code to run
 */
bool newExitCode();

#endif
