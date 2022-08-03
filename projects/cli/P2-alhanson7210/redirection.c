/**
 * @file redirection.c
 */
#include "redirection.h"

/**
 * @param cmds array of commands
 */
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
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
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
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			exit(EXIT_FAILURE);
		}
		close(output_file_fd);
	    return;
     }

     if(execvp(cmds[i].tokens[0], cmds[i].tokens) == -1)
     {
     	perror("execvp");
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
     	exit(EXIT_FAILURE);
     }
}
