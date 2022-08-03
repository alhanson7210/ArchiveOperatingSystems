/**
 * @file built_in_commands.h
 */
#ifndef _BUILT_IN_COMMANDS_H_
#define _BUILT_IN_COMMANDS_H_

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @param command shell command
 */
int handle_built_in_command(char * command);
/**
 * @param command shell command
 * @param delimeter tokenizer delimeter
 */
void run_command(char * command, char * delimeter);

#endif
