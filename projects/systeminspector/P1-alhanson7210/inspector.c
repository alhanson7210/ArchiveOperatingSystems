/**
 * @file
 *
 * System inspector implementation: a Unix utility that inspects the system it
 * runs on and creates a summarized report for the user using the proc pseudo
 * file system.
 *
 * See specification here: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html
 */

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"

#define LNBFSZ 128

/** 
 *Function prototypes 
 */
void print_usage(char *argv[]);
void print_hardware(char * proc);
void print_system(char * proc);
void print_task_list(char * proc);
void print_live_view(char * proc);
void cpu_usage(int * ti);
void memory_usage();
int readline(char *str, ssize_t size, int fd);
char * next_token(char **str_ptr, const char *delim);

/**
 * This struct is a collection of booleans that controls whether or not the
 * various sections of the output are enabled.
 */
struct view_opts {
    bool hardware;
    bool live_view;
    bool system;
    bool task_list;
};
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

/**
 *	readline function for reading characters
 */
int readline(char *str, ssize_t size, int fd)
{
	ssize_t byte_sz = 0;
	char c = 0;
	int idx = 0;

	while( (byte_sz = read(fd, &c, 1) ) > 0 &&  size > 0)
	{
		if(c == '\n')
		{
			*(str + idx) = '\0';
			return byte_sz;
		}

		*(str + idx) = c;
		idx++;
		if(idx == size) return 1;
	}

	if(byte_sz == -1) return -1;
	return byte_sz;
}

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
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
 * Main program entrypoint. Reads command line options and runs the appropriate
 * subroutines to display system information.
 */
int main(int argc, char *argv[])
{
    /* Default location of the proc file system */
    char *procfs_loc = "/proc";

    /* Set to true if we are using a non-default proc location */
    bool alt_proc = false;

    struct view_opts defaults = { true, false, true, true };
    struct view_opts options = { false, false, false, false };

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ahlp:rst")) != -1) {
        switch (c) {
            case 'a':
                options = defaults;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            case 'l':
                options.live_view = true;
                break;
            case 'p':
                procfs_loc = optarg;
                alt_proc = true;
                break;
            case 'r':
                options.hardware = true;
                break;
            case 's':
                options.system = true;
                break;
            case 't':
                options.task_list = true;
                break;
            case '?':
                if (optopt == 'p') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (alt_proc == true) {
        LOG("Using alternative proc directory: %s\n", procfs_loc);

        /* Remove two arguments from the count: one for -p, one for the
         * directory passed in: */
        argc = argc - 2;

	DIR * procdir = opendir(procfs_loc);
        if(procdir == NULL) { printf("Problem opening directory: %s\n", procfs_loc); return EXIT_FAILURE; }
	closedir(procdir);
    }

    if (argc <= 1) {
        /* No args (or -p only). Enable default options: */
        options = defaults;
    }

    if (options.live_view == true) {
        /* If live view is enabled, we will disable any other view options that
         * were passed in. */
        options = defaults;
        options.live_view = true;
        LOGP("Live view enabled. Ignoring other view options.\n");
    } else {
        LOG("View options selected: %s%s%s\n",
                options.hardware ? "hardware " : "",
                options.system ? "system " : "",
                options.task_list ? "task_list" : "");
    }

    //system information: hostname; kernel version; uptime
    if(options.system) {
	print_system(procfs_loc);
    }

    //hardware information: cpu model; processing units; load average (1/5/15 min); cpu usage; memory usage
    if(options.hardware) {
	print_hardware(procfs_loc);
    }

    /**
     * task information:
     * tasks running 
     * pid
     * state
     * task name
     * user
     * tasks
    */
    if(options.task_list) {
	print_task_list(procfs_loc);
    }

    //live cpu/memory view: load average (1/5/15 min); cpu usage; memory usage
    if(options.live_view) {
	print_live_view(procfs_loc);
    }

    return 0;
}















/**
 * printing the hardware estimates
 */
void print_hardware(char * proc)
{
	int read_sz = 0, procstatfd = 0, ldavgfd = 0, cpuinfofd = 0, process_units = 0, start = 0, end = 0, i = 0, j = 0;
	char ln_buf[LNBFSZ] = { 0 }, str_buf[LNBFSZ] = { 0 }, avg_buf[LNBFSZ] = { 0 }, procpath[LNBFSZ] = { 0 };
	char * procfs_loc = proc;
	printf("\nHardware Information\n--------------------\n");

	strcat(procpath, procfs_loc);
	chdir(procpath);

	//cpu model
	cpuinfofd = open("cpuinfo", O_RDONLY);
        while ( (read_sz = readline(ln_buf, LNBFSZ, cpuinfofd)) > 0)
        {
                int current_buffer = 0;
                if( (current_buffer = strncmp(ln_buf, "model name", 10)) == 0 && start == 0)
                {
                        start = strcspn(ln_buf, ":") + 2;
                        end = sizeof(ln_buf);
                        for(i = start; i < end; i++) {str_buf[j] = ln_buf[i]; j++;}
                        printf("CPU Model: %s\n", str_buf);
			break;
                }
        }
        close(cpuinfofd);

	//processing units
	procstatfd = open("stat", O_RDONLY);
        while ( (read_sz = readline(ln_buf, LNBFSZ, procstatfd)) > 0)
	{
		int current_buffer = 0;
		if ( (current_buffer = strncmp(ln_buf, "cpu", 3)) == 0) process_units++;
	}
	process_units--;
	close(procstatfd);
	printf("Processing Units: %d\n", process_units);

	//load avg
        ldavgfd = open("loadavg", O_RDONLY);
        while ( (read_sz = readline(ln_buf, LNBFSZ, ldavgfd)) > 0)
        {
		strncpy(avg_buf, ln_buf, 14);
		printf("Load Average (1/5/15 min): %s\n", avg_buf);
        }
	close(ldavgfd);

	//cpu usage
	int total = 0, idle = 0, cpu = 0, dashes = 0;
	double cpuUsage = 0;
	int ti[2] = { 0 };
	cpu_usage(ti);
	total = ti[0]; 
	idle = ti[1];
	//printf("total = %d idle = %d\n", total, idle);
	cpuUsage = (idle/total) * 100;
	cpu = (int)(round(cpuUsage)/5);
	dashes = (int)(cpu%20);
	//printf("cpuUsage: %f cpu %d dashes %d", cpuUsage, cpu, dashes);
	printf("CPU Usage: [");
	if(cpuUsage <= 0.0) {
		printf("--------------------] %.1lf%% \n", cpuUsage);
	}
	else {	
		int ie;
		for(ie = 0; ie <= cpu; ie++) printf("#");
		for(ie = 0; ie <= dashes; ie++) printf("-");
		printf("] %.1lf%% \n", cpuUsage);
	}
	sleep(1);
	//memory usage
	memory_usage();
}

/**
 * printing system information for kernel
 */
void print_system(char * proc)
{
        //instance variables
        char host_name[LNBFSZ] = { 0 }, kernel_version[LNBFSZ] = { 0 }, uptime_total_secs[LNBFSZ] = { 0 }, line_buf[LNBFSZ] = { 0 }, procpath[LNBFSZ] = { 0 };
        int read_size = 0;
        int hnfd = 0, kvfd = 0, utsfd = 0, ts = 0, ty = 0, td = 0, th = 0, tm = 0, s = 0, y = 31536000, d = 86400, h = 3600, m = 60, x = 0;
	char * procfs_loc = proc;

        //read hostname
        strcat(procpath, procfs_loc);
        strcat(procpath, "/sys/kernel/");
        chdir(procpath);
        hnfd = open("hostname", O_RDONLY);
        while( (read_size = readline(line_buf, LNBFSZ, hnfd)) > 0) strcpy(host_name, line_buf);
        close(hnfd);

        //read kernel version
        chdir(procpath);
        kvfd = open("osrelease", O_RDONLY);
        while( (read_size = readline(line_buf, LNBFSZ, kvfd)) > 0) strcpy(kernel_version, line_buf);
        close(kvfd);

        //print formating for system info for part 1
        printf("System Information\n------------------\nHostname: %s\nKernel Version: %s\n", host_name, kernel_version);

        //uptime switch cases for prints for part 2
        chdir(procfs_loc);
        utsfd = open("uptime", O_RDONLY);
        while ( (read_size = readline(line_buf, LNBFSZ, utsfd)) > 0) strncpy(uptime_total_secs, line_buf, strcspn(line_buf, " "));
        close(utsfd);

        //cconvert time for uptime
        ts = atoi(uptime_total_secs);
        ty = ts/y, td = (ts%y)/d, th = ((ts%y)%d)/h, tm = (((ts%y)%d)%h)/m, s = ((((ts%y)%d)%h)%m)%60;

        printf("Uptime: ");
        (ty == 0)? x++ : printf("%d years, ", ty);
        (td == 0)? x++ : printf("%d days, ", td);
        (th == 0)? x++ : printf("%d hours, ", th);
        printf("%d minutes, ", tm);
        printf("%d seconds\n", s);
}

/**
 * printing the task information of all processes
 */
void print_task_list(char * proc)
{
        printf("\nTask Information\n----------------\n");
	char * procfs_loc = proc;
        int tasks_running = 0;
        chdir(procfs_loc);
        DIR * procdir = opendir(procfs_loc);

        if(procdir == NULL) { printf("Problem opening proc"); return; }

        struct dirent *procentry;
        while ((procentry = readdir(procdir)) != NULL)
        {
                if(atoi(procentry -> d_name) && procentry -> d_type == 4)
                {
                        tasks_running++;
                }
        }
        closedir(procdir);
        free(procentry);
        printf("Tasks running: %d\n", tasks_running);
        //incorrect use of sleep because it doesn't pertain to code yet but it should be used in the project for the cpu usage/ memory usage/ loadavg
	printf("\n%5s | %12s | %25s | %15s | %s \n", "PID", "State", "Task Name", "User", "Tasks");
	printf("------+--------------+---------------------------+-----------------+-------\n");	
}
/**
 * printing out the live view from active data
 */
void print_live_view(char * proc) 
{
	printf("\nTask Information\n----------------\n");
}

/**
 * takes pointers to integers that will contain the total and idle time stored and use them at different times to calculate the percentage of the cpu usage
 */
void cpu_usage(int * ti) 
{
	//cpu usage part 3
	int user = 0, nice = 1, system = 2, id = 3, iowait = 4, irq = 5, softirq = 6, steal = 7, guest = 8, guest_nice = 9,
	tokens = 0, read_sz = 0, procstatfd = 0, total = 0, idle = 0;
	char ln_buf[LNBFSZ] = { 0 };
	procstatfd = open("stat", O_RDONLY);
	while( (read_sz = readline(ln_buf, LNBFSZ, procstatfd)) > 0 )
	{
		char *next = ln_buf, *current_token;
		while ((current_token = next_token(&next, " ")) != NULL) 
		{
			if (!atoi(current_token) && tokens != 9) continue;
			else
			{
				tokens++;
				switch(tokens)
				{
					case 1:
						user = atoi(current_token);
						break;	
					case 2:
                                                nice = atoi(current_token);
                                                break;
					case 3:
                                                system = atoi(current_token);
                                                break;
					case 4:
                                                id = atoi(current_token);
                                                break;
					case 5:
                                                iowait = atoi(current_token);
                                                break;
					case 6:
                                                irq = atoi(current_token);
                                                break;
					case 7:
                                                softirq = atoi(current_token);
                                                break;
					case 8:
                                                steal = atoi(current_token);
                                                break;
					case 9:
                                                guest = atoi(current_token);
						/*char * tmp[LNBFSZ] = { 0 };
						char c = 0;
						int cidx = 0;
						while()*/
                                                break;
					default:
						break;
				}
			}
		}
		break;
	}

	total = user + nice + system + id + iowait + irq + softirq + steal + guest + guest_nice;
	idle = id;
	ti[0] = total; 
	ti[1] = idle;;
}
/**
 * getting the memory info and converting to gb then printing the me info
 */
void memory_usage()
{

	int read_sz = 0, meminfofd = 0;
        char ln_buf[LNBFSZ] = { 0 }, total_buf[LNBFSZ] = { 0 }, active_buf[LNBFSZ] = { 0 }, 
	memTotal[LNBFSZ] = "MemTotal", memActive[LNBFSZ] = "Active:";
	meminfofd = open("meminfo", O_RDONLY);
	//goes to the end of the file
	while( (read_sz = readline(ln_buf, LNBFSZ, meminfofd)) > 0)
	{
		int buf = 0;
		char *next = ln_buf, *current_token = ln_buf;
                if((buf = strncmp(ln_buf, memTotal, 8)) == 0)
                {
			//goes to the end of the string
			while ((current_token = next_token(&next, " ")) != NULL)
                	{
				if(atoi(current_token)) {
					strcpy(total_buf, current_token);							
				}
                	}
                }
                if ((buf = strncmp(ln_buf, memActive, 7)) == 0)
                {
			while ((current_token = next_token(&next, " ")) != NULL)
                	{
                                if(atoi(current_token)) {
					strcpy(active_buf, current_token);
                                }
                	}
                }
	}
	if(atoi(active_buf) && atoi(total_buf))
	{
		int active = 0, total = 0;
		float act = 0.0, tot = 0.0, per = 0.0;

		active = atoi(active_buf); 
		total = atoi(total_buf);

		act = (active/1024)/1024; 
		tot = (total/1024)/1024;
		per = (active/total) * 100;

		printf("Memory Usage: [-------------------] %.1f%% (%.1f GB / %.1f GB)\n", per, act, tot);
	}
	else { printf("Memory Usage: [-------------------] 0.0%% (0.0 GB / 0.0 GB)\n");}
	close(meminfofd);
}
