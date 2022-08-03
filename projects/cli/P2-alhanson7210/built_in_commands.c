/**
 * @file built_in_commands.c
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> 
#include <pwd.h>
#include <sys/wait.h> 
#include <limits.h>
#include "built_in_commands.h"
#include "string_functions.h"
#include "history.h"
#include "prompt.h"
#include "debug.h"

/**
 * @param command get line command
 */
int handle_built_in_command(char * command)
{
	if(*command == '#')
	{
		add_entry(get_cmd(), command);
		increment_cmd();
		return 1;
	}
	char c = ' ';
	int sp = 0, i = 0;
        while( (c = *(command  + i)) != '\0' && sp < 4096) 
	{
		if(c == ' ') sp++;
		i++;
	}
	if(sp == 4095)
	{
		LOGP("checking command but it was too long\n");
		return 1;
	}

	LOG("Received %s\n", command);
	if(strcmp(command, "history") == 0) 
	{
		LOGP("ADDING BUILT IN TO THE HISTORY\n");
		add_entry(get_cmd(), command);
		LOGP("printing history\n");
		print_history();
		return 0;
	}
	if(strncmp(command, "setenv", 6) == 0)
	{
		do
		{
			LOG("ADDING BUILT IN TO THE HISTORY: %s\n", command);
			add_entry(get_cmd(), command);
			char * se = strdup(command);
			char * env[4096] = {0};
			LOGP("tokenizing the setenv command\n");
			int var = pipe_command_tokenizer(se, env, " \0");
			if(var != 3)
			{
				LOGP("there was not three values\n");
				//free(se);
				break;
			}
			if(setenv(env[1], env[2], 1) != 0)
			{
				LOGP("unsuccessful setenv call\n");
				//free(se);
				break;
			}
			LOGP("set env var\n");
			//free(se);
			return 0;
		} while(0);
	}
	do {
		if(*command == '!')
		{
        		if(strcmp(command, "!!") == 0)
        		{
                		LOGP("Printing the last command from !!\n");
                		run_last_history_command();
				LOGP("This command is a !! command\n");
                		return 0;
        		}
			LOGP("Not a !! command\n");
			char num[64] = {0}, c;
			int i = 1;
			LOGP("starting bang loop check\n");
			while( (c = *(command  + i)) != '\0' || i != 65)
			{
				if(c != '\n' || c != ' ' || c != '\t' || c != '\r') num[i-1] = c;
				i++;
			}
			if(atoi(num))
			{
				LOGP("Numbered bang command found\n");
				run_command_id(atoi(num));
				return 0;
			}
			LOGP("Not a bang command number\n");
			char * prefix = strdup(num);
			if(run_prefix_command(prefix) == -1) 
			{
				LOGP("Not a band prefix command\n");
				break;
			}
			LOGP("exiting the bang check\n");
			free(prefix);
			return 0;
		}
	} while(0);
	return -1;
}
/**
 * @param command get line command
 * @param delimeter token delimeter
 */
void run_command(char * command, char * delimeter)
{
	char c = ' ';
        int sp = 0, i = 0;
        while( (c = *(command  + i)) != '\0' && sp < 4096)
        {
                if(c == ' ') sp++;
                i++;
        }
        if(sp == 4096) return;

	add_entry(get_cmd(), command);
	LOGP("successfully added entry\n");

	LOGP("Getting rid of any comments\n");
	char * p_c = strdup(command);
	char ** execvp_command = NULL;
	p_c = next_token(&p_c, "#");
	if(*p_c == ' ') return;

	char * buffer[4096] = {0};
	int pipes = pipe_command_tokenizer(p_c, buffer, "|\0");
	LOG("pipes counted %d\n", pipes-1);
	if(pipes-1 != 0)
	{
		LOG("pipes counted %d\n", pipes-1);
		return;
	}
	LOGP("starting the tokenization\n");
	execvp_command = command_tokenizer(p_c, delimeter);
	LOGP("successfully tokenized the command\n");
	pid_t pid = fork();
	if (pid == -1) 
	{ 
		LOGP("fork failed\n");
		perror("fork");
		exit(EXIT_FAILURE);
	}
	LOGP("Forked successfully\n");
	if (pid == 0) 
	{ 
		LOG("Executing command %s\n", p_c); 
		if( (execvp(execvp_command[0], execvp_command)) == -1) 
		{ 
			setExitCode(true);
			LOGP("execvp failed\n");
			perror("execvp");
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			exit(EXIT_FAILURE);
		}
		LOGP("freed the memory for the execvp_command");
		free(execvp_command);
		LOGP("didn't fail to free execvp_command\n");
		exit(EXIT_FAILURE);
	}
	else 
	{ 
		int status;
		LOG("waiting for %d\n", pid);
		wait(&status);
	}
	free(p_c);
	LOG("Received %s\n", command);
}
