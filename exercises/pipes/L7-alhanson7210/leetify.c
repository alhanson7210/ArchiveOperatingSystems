#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
//#define SIZE 1024

struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
};

void execute_pipeline(struct command_line *cmds)
{
    /**
     * TODO: design an algorithm that sets up a pipeline piece by piece.
     * Solutions will probably either iterate over the pieces of the pipeline or
     * work recursively. Imagine you have three commands in a pipeline:
     *
     *  - cat
     *  - tr
     *  - sed
     *
     * Use 'stdout_pipe' to determine when you've reached the last command in
     * the pipeline, and 'stdout_file' to decide whether the final result gets
     * written to a file or the terminal.
     *
     * If we aren't at the last command, then we need to set up a pipe for the
     * current command's output to go into. For example, let's say our command
     * is `cat file.txt`. We will create a pipe and have the stdout of the
     * command directed to the pipe. Before running the next command, we'll set
     * up the stdin of the next process to come from the pipe, and
     * execute_pipeline will run whatever command comes next (for instance,
     * `tr '[:upper:]' '[:lower:]'`).
     *
     * Here's some pseudocode to help:
     *
     * create a new pipe
     * fork a new process
     * if pid is the child:
     *     dup2 stdout to pipe[1]
     *     close pipe[0]
     *     execvp the command
     * if pid is the parent:
     *     dup2 stdin to pipe[0]
     *     close pipe[1]
     *     move on to the next command in the pipeline
     *
     * The special case here is when there are no more commands left. In that
     * case, you can simply execvp the command (no need to create another pipe).
     * If you created a handler process in main(), then it will be replaced by
     * this last call to execvp.
     */
     int i = 0;
     int fd[2];
     
     while(cmds[i].stdout_pipe)
     {
	     //did tthe pipe break for some reason?
	     if(pipe(fd) == -1)
	     {
	     	perror("pipe");
	     	exit(EXIT_FAILURE);
	     }

		 //split the processes
	     pid_t pid = fork();
	     if(pid == -1)
	     {
	     	perror("fork");
	     	exit(EXIT_FAILURE);
	     }
	     //child process that executes the command
	     if(pid == 0)
	     {
	     	close(fd[0]);
	     	if(dup2( fd[1], STDOUT_FILENO) == -1) 
	     	{
	     		perror("dup2");
	     		exit(EXIT_FAILURE);
	     	}
	     	
	     	if(execvp(cmds[i].tokens[0], cmds[i].tokens) == -1)
	     	{
	     		perror("execvp");
	     		exit(EXIT_FAILURE);
	     	}
	     }
	     //parent process that accepts input from the child for the next command
	     else
	     {
	     	close(fd[1]);
	     	if(dup2(fd[0], STDIN_FILENO) == -1)
	     	{
	    		perror("dup2");
	    		exit(EXIT_FAILURE);
	     	}
	     	i++;
	     }
     }
     //last command
     if(cmds[i].stdout_file != NULL)
     {
     	int open_flags = O_WRONLY | O_CREAT | O_TRUNC, open_permissions = 0644, output_file_fd = open(cmds[i].stdout_file, open_flags, open_permissions);

	    if(output_file_fd == -1)
	    {
	    	perror("open");
	    	exit(EXIT_FAILURE);
	    }
	    
	    if(dup2(output_file_fd, STDOUT_FILENO) == -1)
	    {
	    	perror("dup2");
	    	exit(EXIT_FAILURE);
	    }
	    
		if(execvp(cmds[i].tokens[0], cmds[i].tokens) == -1)
		{
			perror("execvp");
			exit(EXIT_FAILURE);
		}
		close(output_file_fd);
	    return;
     }

     if(execvp(cmds[i].tokens[0], cmds[i].tokens) == -1)
     {
     	perror("execvp");
     	exit(EXIT_FAILURE);
     }
}

int main(int argc, char *argv[])
{
    char *input_file = NULL;
    char *output_file = NULL;

    if (argc < 2 || argc > 3) {
        printf("Usage: %s file-to-leetify [output-file]\n", argv[0]);
        return 1;
    }

    input_file = argv[1];

    if (argc == 3) {
        output_file = argv[2];
    }

    /* TODO: Next steps:
     *   finish setting up the array of command_line structs
     *   fork() to create a 'handler' process that will call execute_pipeline
     *   wait for the handler to finish
     */

    // TODO: These commands aren't quite finished...
    char *command1[] = { "cat", input_file, (char *) NULL };
    char *command2[] = { "tr", "[:upper:]", "[:lower:]", (char *) NULL };
    char *command3[] = { "sed", "s|CATS|MEOW|g; s|the|teh|g; s|a|4|g; s|e|3|g; s|i|!|g; s|l|1|g; s|o|0|g; s|s|5|g;", (char *) NULL };

    struct command_line cmds[3] = { 0 };
    
    cmds[0].tokens = command1;
    cmds[0].stdout_pipe = true;
    cmds[0].stdout_file = NULL;

    cmds[1].tokens = command2;
    cmds[1].stdout_pipe = true;
    cmds[1].stdout_file = NULL;
    
    /* TODO: Finish filling these in ... */
	cmds[2].tokens = command3;
    cmds[2].stdout_pipe = false; /* Last command so set stdout_pipe = false */
	cmds[2].stdout_file = output_file;

	pid_t p = fork();
	if(p == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	
	int status = 0;
	if(p == 0) execute_pipeline(cmds);
	else wait(&status);
	
    return 0;
}
