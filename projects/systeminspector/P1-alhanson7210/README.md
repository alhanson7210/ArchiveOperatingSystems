## Project 1: System Inspector
See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html

*Writer: Alex Hanson*

University of San Francisco

CS326 Operating Systems

## About This Project:
> This is a Unix utility that inspects the system it runs on and creates a summarized report for the user. 
> Similar to the top command, this utility prints system, hardware, tasks, and processes information in an organized
> and easy to understand format. This information is retrieved by using system calls to read files from proc, the process 
> information pseudo-filesystem.

## What is proc:
> proc is the process information pseudo-filesystem. Inside /proc, there are process IDs and several other virtual files 
> that are updated dynamically with system information. Each of the system processes corresponds to one of the numbered 
> directories in /proc. To get a better understanding of proc, check out the man page: man proc. The manual has a complete 
> description of every file and directory stored under /proc

## Program Options 
```
/**
 * Prints help/program usage information.
 *
 * This output is displayed if there are issues with command line option parsing
 * or the user passes in the -h flag.
 */
void print_usage(char *argv[])
{
    printf("Usage: %s [-ahrst] [-l] [-p procfs_dir]\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
"    * -a              Display all (equivalent to -rst, default)\n"
"    * -h              Help/usage information\n"
"    * -l              Live view. Cannot be used with other view options.\n"
"    * -p procfs_dir   Change the expected procfs mount point (default: /proc)\n"
"    * -r              Hardware Information\n"
"    * -s              System Information\n"
"    * -t              Task Information\n");
    printf("\n");
}
```
>

To compile and run:

```bash
make
./inspector
```

## Program Output

```
System Information
------------------
Hostname: alhanson7210
Kernel Version: 5.2.9-arch1-1-ARCH
Uptime: 21 days, 9 hours, 35 minutes, 38 seconds

Hardware Information
--------------------
CPU Model: AMD EPYC Processor (with IBPB)
Processing Units: 2
Load Average (1/5/15 min): 0.00 0.00 0.00
CPU Usage: [--------------------] 0.0%
Memory Usage: [-------------------] 0.0% (0.0 GB / 0.0 GB)

Task Information
----------------
Tasks running: 87

  PID |        State |                 Task Name |            User | Tasks
------+--------------+---------------------------+-----------------+-------
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
