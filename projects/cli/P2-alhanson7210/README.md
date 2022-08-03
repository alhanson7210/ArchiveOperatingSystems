# Project 2: Command Line Shell

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-2.html

*Writer: Alex Hanson*

University of San Francisco

CS326 Operating Systems

## About This Project:
> The outermost layer of the operating system kernel is called the shell. In Unix-based systems, the shell is generally a command line interface. 
> Most Linux distributions ship with bash as the default (there are several others: csh, ksh, sh, tcsh, zsh). In this project, what will  be implementing is a shell.

## Program options:
```
Upon startup, the shell will print its prompt and wait for user input. The shell should be able to run commands in both the current directory and those in the 
PATH environment variable. Execvp was used to do this. To run a command in the current directory, you’ll need to prefix it with ./ as usual. If a command isn’t found, it prints an error message:
```

To compile and run:

```bash
make
./flash
```

## Program Output
```
Command 417
Command 418
Command 419
-----
background_jobs.c    built_in_commands.o  history.c  prompt.c	redirection.c	     scripting.h	signal_handling.h   string_functions.o	variable_expansion.c
background_jobs.h    debug.h		  history.h  prompt.h	redirection.h	     shell.c		signal_handling.o   test-output.md	variable_expansion.h
built_in_commands.c  docs		  history.o  prompt.o	script_commands.txt  shell.o		string_functions.c  tests		working_directory.c
built_in_commands.h  flash		  Makefile   README.md	scripting.c	     signal_handling.c	string_functions.h  test.txt		working_directory.h
execvp: No such file or directory
alhanson7210
alhanson pts/0        2019-11-02 09:08 (192.168.122.1)
execvp: No such file or directory
-----
0 ls
1 echo Command 388
2 echo Command 389
3 echo Command 390
4 echo Command 391
5 echo Command 392
6 cat /etc/hostname
7 echo Command 394
8 echo Command 395
```

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```