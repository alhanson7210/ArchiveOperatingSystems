/**
 * @file string_functions.h
 */
#ifndef _STRING_FUNCTIONS_H_
#define _STRING_FUNCTIONS_H_

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @param void get shell name
 */
char * get_shell_name();

/**
 * @param void get shell command for interactive mode
 */
char * get_shell_command();

/**
 * @param str_ptr string to tokenize
 * @param delim delimeter
 */
char * next_token(char **str_ptr, const char *delim);

/**
 * @param command shell command
 * @param delimeter string delimeter
 */
char ** command_tokenizer(char * command, char * delimeter);

/**
 * @param void get line function for shell
 */
char * get_script_command();

/**
 * @param command shell command
 * @param pipe_buffer pipe commands
 * @param delim delimeter
 */
int pipe_command_tokenizer(char * command, char ** pipe_buffer, char * delimeter);

#endif
