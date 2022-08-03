/**
 * @file string_functions.c
 */
#include "string_functions.h"
#include "debug.h"

#include <stdlib.h>

#define BUFFERSIZE 64
#define COMMANDSIZE 4096

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * @param str_ptr :maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * @param delim :the set of characters to use as delimiters
 *
 * Returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

/**
 * @param void return the name of the shell
 */
char * get_shell_name()
{
	static char shellname[BUFFERSIZE] = "flash";
	
	return shellname;
}

/**
 * @param void get shell command
 */
char * get_shell_command()
{
	LOGP("Setting up command entry\n");
	char *line = NULL, *nl = "\n";
	size_t line_sz = 0;

	LOGP("Preparing to get shell command\n");
	getline(&line, &line_sz, stdin);
	if(line_sz > 4095)
	{
		line = strdup(nl);
		LOGP("command was over 4095\n");
		return line;
 	}
	LOG("read line from stdin: %s\n", line);
	return line;
}

/**
 * @param get script command from stdin
 */
char * get_script_command()
{
		LOGP("Setting up command entry\n");
		char *line = NULL, *nl = "\n";
		size_t line_sz = 0;
	
		LOGP("Preparing to get shell command\n");
		if(getline(&line, &line_sz, stdin) == -1) 
		{
			free(line);
			LOGP("get line failed\n");
			return NULL;
		}
		if(line_sz > 4095) 
		{
			line = strdup(nl);
			LOGP("command was over 4095\n");
			return line;
		}
		LOG("read line from stdin: %s\n", line);
		return line;
}

/**
 * @param command command to be tokenized to run for execvp and the command_line struct
 * @param delimeter tokenizing string
 */
char ** command_tokenizer(char * command, char * delimeter)
{
	LOGP("SETTING VARIABLES\n");
	int tokens = 0;
	static char ** tok = NULL;
	char *tokenized[COMMANDSIZE] = { 0 };
	char * current_token = NULL;

	char next_token_string[COMMANDSIZE] = {0};
	char * nxt = next_token_string;

	LOGP("copying command\n");
	//strcpy(nxt, command);
	nxt = strdup(command);
	LOGP("strcpy no longer failing\n");
	while((current_token = next_token(&nxt, delimeter)) != NULL && tokens < COMMANDSIZE) 
	{
		LOG("tokenized command %s\n", current_token);
		tokenized[tokens++] = strdup(current_token);
		if(tokens == COMMANDSIZE) tokenized[tokens-1] = '\0';
	}
	free(current_token);
	//free(next_token_string);
	tok = tokenized;
	return tok;
}

/**
 * @param command get line command
 * @param pipe_buffer pipe command tokenizer buffer of commands
 * @param delimeter string tokenizer
 */
int pipe_command_tokenizer(char * command, char ** pipe_buffer, char * delimeter)
{
	LOGP("SETTING VARIABLES\n");
	int tokens = 0;
	char * current_token = NULL;
	LOGP("copying command\n");
	//strcpy(nxt, command);
	char * nxt = strdup(command);
	LOGP("strcpy no longer failing\n");
	while((current_token = next_token(&nxt, delimeter)) != NULL && tokens < COMMANDSIZE) 
	{
		LOG("tokenized command %s\n", current_token);
		pipe_buffer[tokens++] = strdup(current_token);
		if(tokens == COMMANDSIZE) pipe_buffer[tokens-1] = '\0';
	}
	free(current_token);
	free(nxt);
	//free(next_token_string);
	return tokens;
}
