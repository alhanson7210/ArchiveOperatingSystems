#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* Function prototypes */
void initialize(void) __attribute__((constructor));
int open(const char *pathname, int flags);

/* Function pointers */
int (*orig_open)(const char * pathname, int flags) = NULL;

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

void initialize(void)
{
    orig_open = (int (*)(const char *, int)) dlsym(RTLD_NEXT, "open");
    LOG("Original open() location: %p\n", orig_open);
    LOG("New open() location: %p\n", open);
}

int open(const char *pathname, int flags)
{

    int errnum;
    char *rollpathname = "/roll.txt";
    char cwd[1024];

    //the developer could use the strlen of pathname to ensure that you get the last '.' instead of using strcspn instead
    if(strncmp(&pathname[strcspn(pathname, ".")], ".java", 5) == 0)
    {    errno = ENOENT;
        errnum = errno;
        LOG("Path to %s\n does not exist\n", pathname);
    fprintf(stderr, "Error printed from opening file: %s\n", strerror(errnum));
        return -1;
    }

    if(strncmp(&pathname[strcspn(pathname, ".")], ".txt", 4) == 0)
    {
        getcwd(cwd,sizeof(cwd));
        strcat(cwd,rollpathname);
        LOG("Opening file(after checking the extension): %s\n", pathname);
        //free(rollpathname);
        return (*orig_open)(cwd, flags);
    }

    getcwd(cwd,sizeof(cwd));
    strcat(cwd,rollpathname);
    LOG("Opening file(no extension check): %s\n", rollpathname);
    //return (*orig_open)(pathname, flags);
    return (*orig_open)(cwd, flags);
}
