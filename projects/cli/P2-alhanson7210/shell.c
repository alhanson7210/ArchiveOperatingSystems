/**
 * @file shell.c
 */

#include <fcntl.h>
#include <locale.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/limits.h>

#include "debug.h"
#include "history.h"
#include "prompt.h"
#include "signal_handling.h"
#include "built_in_commands.h"
#include "string_functions.h"

/**
 * @param argc number of arguments
 */
int main(int argc, char ** argv)
{
    char shellname[] = "shellname";

    LOGP("Initializing shell\n");
    char *locale = setlocale(LC_ALL, "en_US.UTF-8");

    LOG("Setting locale: %s\n",
            (locale != NULL) ? locale : "could not set locale!");

    LOGP("Preparing print history function\n");
    create_history_log();

    LOGP("Testing the print shell prompt function worked so code was removed\n");

    LOGP("Creating exit signals\n");
    create_signals();
    char * newline = "\n";
    char * delimeter = " \t\n\r";
    char * command_line_input;
    int exitcode = 0;
    LOGP("Starting the shell runtime envirnonment\n");
    if (isatty(STDIN_FILENO)) {
        LOGP("stdin is a TTY; entering interactive mode\n");
       	while(true) 
       	{
       		print_shell_prompt(shellname, exitcode);
       		exitcode = 0;
                setExitCode(false);
       		command_line_input = get_shell_command();
       		if(strcmp(command_line_input, "\n") == 0) {free(command_line_input); continue;}
       		command_line_input = next_token(&command_line_input, newline);
       		if(strcmp(command_line_input, "exit") == 0) break;
       		else if(strcmp(command_line_input, "\0") == 0) continue;
                else if(strcmp(command_line_input, "^C") == 0)
		{
                    printf("\n");
                    fflush(stdout);
                    setExitCode(false);
                    continue;
                }
                else if(strcmp(command_line_input, newline) == 0)
       		{
                    exitcode = 0;
                    fflush(stdout);
                    printf("\n");
                    continue;
                }
                else
                {
                    //the built in was ran instead if it != -1 
                    int h = handle_built_in_command(command_line_input);
                    if(h < 0)
                    {
                        //use execvp
                        LOGP("checking for runnable\n");
                        run_command(command_line_input, delimeter);
                        if(!newExitCode()) exitcode = 0;
                    }
                    else if(h > 0) 
                    {
                        LOGP("Command was too long\n");
                        continue;
                    }
                    increment_cmd();
                }
       		LOG("%s\n", command_line_input);
        }
    } 
    else {
        LOGP("data piped in on stdin; entering script mode\n");
    	if(argc == 3)
    	{
    		int arg_fd,  open_flags = O_WRONLY | O_CREAT | O_TRUNC, open_permissions = 0644;
    		
    		if((arg_fd = open(argv[2], open_flags, open_permissions)) == -1) 
    		{
    			perror("open");
    			exit(EXIT_FAILURE);
    		}
    		
    		if(dup2(arg_fd, STDOUT_FILENO) == -1)
    		{
    			perror("dup2");
    			exit(EXIT_FAILURE);
    		}
    		close(arg_fd);
    	}
       	while(true) 
       	{
			char * command_line_input = get_script_command();
			//if(strcmp(command_line_input, "\n") == 0) {free(command_line_input); continue;}
       		//command_line_input = next_token(&command_line_input, newline);
       		//if(strcmp(command_line_input, "\n") == 0) {free(command_line_input); continue;}
       		if(feof(stdin))
       		{
       			free(command_line_input);
       			LOGP("end of file\n");
       			break;
			}
       		if(command_line_input == NULL)
       		{
       			free(command_line_input);
       			LOGP("Get line may have failed from a huge command\n");
       			break;
       		}
       		if(strcmp(command_line_input, "exit\n") == 0)
       		{
       			free(command_line_input);
       			LOGP("Exiting Normally with exit\n");
       			break;
       		}
       		if(strcmp(command_line_input, "\n") == 0) 
       		{
       			free(command_line_input); 
       			continue;
       		}
       		command_line_input = next_token(&command_line_input, newline);
       		//command_line_input = get_script_command();
       		if(strcmp(command_line_input, newline) == 0)
       		{
       			exitcode = 0;
       			continue;
       		}
       		else 
       		{
       			//the built in was ran instead if it != -1 
       			int h = handle_built_in_command(command_line_input);
				if(h < 0)
       			{
       				//use execvp
       				LOGP("checking for runnable\n");
       				run_command(command_line_input, delimeter);
       				if(!newExitCode()) exitcode = 0;
       			}
				else if(h > 0) 
				{
					LOGP("Command was too long\n");
					continue;
				}
       			increment_cmd();
       		}
       		LOG("%s\n", command_line_input);
       	}
    }
	//shaded emoji
    //char s[] = { 0xf0, 0x9f, 0x98, 0x8e, 0};
    //printf("\n%s\n", s);
    //smiley face
    //printf("\u263A\n\n");
/*
next_token(){...}
char s[128] = "shellname"
while (true)
{
	print_shell_prompt(s);
	char *line = NULL;
	size_t line_sz = 0;
	getline(&line, &line_sz, stdin);
	LOG("read line from stdin: %s", line);

	char *line_dup = strdup(line);

	char *args[10]; //ARG_MAX INSTEAD OF TEN
	int tokens = 0;
	char * next_tok = line;
	char * current_token;
	while( (current_token = next_token(&next_tok, " \t\r\n")) = != NULL)
	{
		args[tokens++] = current_token;
		//ignore spaces in the history
		//macro(no overhead) vs function
	}
	args[tokens] = 0;

	if(args[0] == 0) continue;

	//handle_built_in_commands() - handles the built in commands and other base cases
	if(strcmp("exit", args[0]) == 0) ...break... exit(EXIT_SUCCESS);
	
	pid_t child = fork();
	if (child == -1) perror("fork");
	else if(child == 0) 
	{
		//child - runs a process: execvp
		//char *args[] = { "ls", 0};
		int ret = execvp(args[0], args);
		if (ret == -1) perror("execvp");
		exit(EXIT_FAILURE); //GETS RID OF ZOMBIE CHILDREN
	}
	else 
	{
		//parent - printer
		//waitpid(-1, pointer to an integer, 0)
		//int status;
		//waitpid(childd, &status, 0);
	}
	free(line_dup);
	free(line);
}
*/	
    return 0;
}
