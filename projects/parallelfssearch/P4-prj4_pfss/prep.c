/**
 * @file prep.c
 *
 * Unix utility that recursively searches for matching words in text files,
 * similar to the grep command (specifically with flags -Rnw). Only entire words
 * are matched, not partial words (i.e., searching for 'the' does not match
 * 'theme'). The line number where the matching search term was found is also
 * printed.
 *
 * The tool makes use of multiple threads running in parallel to perform the
 * search. Each file is searched by a separate thread.
 */

#define _GNU_SOURCE

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"

/* Preprocessor directives */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define FILE_SIZE_MAX _POSIX_ARG_MAX

/* Function prototypes */
void print_usage(char *argv[]);
void recurse_directory(struct dirent * directory_entry);
void * thread_parse_file(void * search_file);
char * line_parser(char * line, size_t size);
bool case_sensitivity_matching(char * word);
char * permission_decyper(char * absolute_path, struct dirent * entry);
void print_line_diagnostics(char * absolute_path, int line_number, char * permissions, char * line);
char * next_token(char **str_ptr, const char *delim);
char * strip_line(char * line, size_t size);

/* Globals */
char *g_search_dir = NULL;
bool g_exact = false;
unsigned int g_num_terms = 0;
char **g_search_terms;
sem_t main_thread;

/**
 * @param argv command line
 * @brief printing command usage usage
 */
void print_usage(char *argv[])
{
    fprintf(stdout, "Usage: %s [-eh] [-d directory] [-t threads] "
            "search_term1 search_term2 ... search_termN\n" , argv[0]);
    fprintf(stdout, "\n");
    fprintf(stdout, "Options:\n"
           "    * -d directory    specify start directory (default: CWD)\n"
           "    * -e              print exact name matches only\n"
           "    * -h              show usage information\n"
           "    * -t threads      set maximum threads (default: # procs)\n");
    fprintf(stdout, "\n");
}

/**
 * @param argc
 * @param argv
 * @brief functionality mainframe
 */
int main(int argc, char *argv[])
{
    if(argc == 1) 
    {
        print_usage(argv);
        return 1;
    }

    int max_threads = get_nprocs();

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "d:eht:")) != -1) {
        switch (c) {
            case 'd':
                g_search_dir = optarg;
                break;
            case 'e':
                g_exact = true;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            case 't':
                max_threads = atoi(optarg);
                break;
            case '?':
                if (optopt == 'd' || optopt == 't') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                return 1;
            default:
                abort();
        }
    }

    g_num_terms = argc - optind;
    LOG("number of options %d\n", optind);
    LOG("first term starts at %s\n", argv[optind]);
    LOG("last term starts at %s\n", argv[argc-1]);
    g_search_terms = &argv[optind];

    if(max_threads <= 0) 
    {
        print_usage(argv);
        return 1;
    }

    if(!g_search_dir) g_search_dir = getcwd(g_search_dir, PATH_MAX);

    chdir(g_search_dir);

    DIR * directory = opendir(g_search_dir);
    if(!directory) 
    {
        LOG("%s is the directory that failed\n", g_search_dir);
        perror("opendir");
        return 1;
    }


    LOG("Searching '%s' recursively using %d threads\n", g_search_dir, max_threads);
    sem_init(&main_thread, 0, max_threads);

    LOGP("initialized the main thread and starting to recurse given search directory\n");
    struct dirent * entry = NULL;
    while((entry = readdir(directory)) != NULL)
    {
        LOG("Recursing directory entry: %s; \nd_type: %d; \nDT_REG: %d; \nDT_DIR: %d\n", entry -> d_name, entry -> d_type, DT_REG, DT_DIR);
        if( (strcmp(entry -> d_name, ".") && strlen(entry -> d_name) == 1) || ( strcmp(entry -> d_name, "..") && strlen(entry -> d_name) == 2) ) continue;
        recurse_directory(entry);
    }
    closedir(directory);

    LOGP("Finished recursing directory for files\n");
    sem_destroy(&main_thread);
    pthread_exit(0);
    
    LOGP("Threads all working threads finished parsing files before the main thread ended its execution\n");
    return 0;
}





































//recursive functions

/**
 * @param directory_entry
 * @brief 
 */
void recurse_directory(struct dirent * directory_entry)
{
    if(directory_entry -> d_type & DT_REG)
    {
        LOG("got file %s\n", directory_entry -> d_name);
        char truncated_file_path[PATH_MAX] = {0};
        char *file_path = realpath(directory_entry -> d_name, truncated_file_path);

        pthread_t worker_thread;
        int ret = pthread_create(&worker_thread, NULL, thread_parse_file, (void *) file_path);
        if (ret != 0)
        {
            LOGP("worker thread failed to create and parse file\n");
            perror("pthread_create");
        }

    }

    else if (directory_entry -> d_type & DT_DIR)
    {
        LOG("recurse on a sub-directory %s\n", directory_entry -> d_name);

        char truncated_file_path[PATH_MAX] = {0};
        char *file_path = realpath(directory_entry -> d_name, truncated_file_path);

        LOG("retrieved real path from file system call; file_path: %s; truncated: %s\n", file_path, truncated_file_path);
 
        DIR * sub_directory = opendir(file_path);
        if(!sub_directory)
        {
            LOG("%s is the directory failed\n", file_path);
            perror("opendir");
            return;
        }
        struct dirent * sub_directory_entry = NULL;
        while( (sub_directory_entry = readdir(sub_directory)) != NULL)
        {
            LOG("Sub directory entry as %s to recurse\n", sub_directory_entry -> d_name);
            if( strcmp(sub_directory_entry -> d_name, ".") || strcmp(sub_directory_entry -> d_name, "..") ) continue;
            recurse_directory(sub_directory_entry);
        }
        closedir(sub_directory);
    }

    return;
}

/**
 * @param search_file
 * @brief 
 */
void * thread_parse_file(void * search_file)
{
    //separate thread from main thread as a working thread and decrement semaphore
    pthread_detach(pthread_self());
    LOGP("detached thread from main thread as a working thread\n");

    sem_wait(&main_thread);
    LOGP("decrement semaphore for this working thread\n");

    /*parsing section*/
     struct dirent * file_entry = (struct dirent *) search_file;
     char * sym = (char *) search_file;
    
    //getting file path
    char truncated_file_path[PATH_MAX] = {0};
    char *file_path = realpath(sym, truncated_file_path);

    LOG("retrieved real path from file system call; file_path: %s; truncated: %s\n", file_path, truncated_file_path);

    // Opening file
    FILE * file = fopen(file_path, "r");
    LOGP("opened the file\n");
    if (!file) 
    {
        LOG("file path is probably null: %s\n", file_path);
        perror("fopen");
        sem_post(&main_thread);
        return (void *)1;
    }
    
    //local instances
    int lines = 1;
    char *line = NULL,
    *parsed_line = NULL,
    *permissions = NULL,
    *stripped_line = NULL;
    size_t line_size = 0;

    //reading lines
    LOGP("reading lines from the file\n");
    while (getline(&line, &line_size, file) != -1)
    {
        //parse characters in line
        *(line + strcspn(line, "\n")) = '\0';
        LOG("unparsed line should be %s", line);

        //TODO: finish line parser
        parsed_line = line_parser(line, line_size);
        char * space = " ",
        *word = NULL;
        LOG("finished the line parser %s\n", parsed_line);

        //parse words in line
        while((word = next_token(&parsed_line, space)) != NULL)
        {
            //handles case sensitivity and matches
            if(case_sensitivity_matching(word))
            {
                //decyper file permissions
                //TODO: finish decyper
                LOG("Word match %s was found and permission checking is starting\n", word);
                permissions = permission_decyper(file_path, file_entry);
                LOGP("finished decypering the files permissions\n");

                //strip left space of the line
                LOGP("start stripping left/right space of the line\n");
                stripped_line = strip_line(line, line_size);
                LOG("newly stripped line is %s\n", stripped_line);

                //print line diagnostics
                print_line_diagnostics(truncated_file_path, lines, permissions, stripped_line);
                LOGP("done printing the diagnostics\n");
                break;
            }
        }
        lines++;
    }

    //close file
    LOGP("closing file\n");
    fclose(file);
    free(line);
    free(permissions);
    /*parsing section*/

    //increment semaphore
    sem_post(&main_thread);
    return (void *)0;
}

//string manipulation functions

/**
 * @param line
 * @param size
 * @brief
 */
char * strip_line(char * line, size_t size)
{
    LOG("line in strip line is %s\n", line);
    LOG("size is %ld\n", size);
    char * stripped_line = strdup(line),
    search_char = ' ';

    int i = 0, end = strlen(stripped_line) - 1, full_size = strlen(line)-1;
    search_char = *(stripped_line + end);

    LOG("full size should be %d\n", full_size);

    while(search_char == ' ')
    {
        end--;
        full_size--;
        search_char = *(stripped_line + end);
    }
    LOG("end index is %d\n", end);

    *(stripped_line + end + 1) = '\0';

    search_char = *(stripped_line + i);
    while(isspace(search_char))
    {
        i++;
        full_size--;
        search_char = *(stripped_line + i);
    }
    
    char * fl = (char *) calloc(1, full_size);
    char * l = &stripped_line[i];

    strncat(fl, l, full_size);
    LOG("strncat line : %s\n", fl);
    int u = strlen(fl) - 1;
    char f = *(fl + u);
    while(isspace(f))
    {
        u--;
        f = *(fl + u);
    }

    char * flu = (char *) calloc(1, u);
    strncat(flu, fl, u+1);
    free(fl);
    return strdup(flu);
/*

    LOGP("copying the string from stripped_line into fixed_line\n");
    LOG("full size should be %d\n", full_size);

    while(search_char != '\0' && z != full_size)
    {
        z++;
        i++;
        *(fixed_line + z) = *(stripped_line + i);
    }

    LOG("stripped_line is %s\n", fixed_line);

    return strdup(fixed_line);*/
}

/**
 * @param line
 * @param size
 * @brief
 */
char * line_parser(char * line, size_t size)
{
    char * parsed_line = (char *) calloc(1, size),
    *delim = "\t\r\n.,:;?!`()[]{}|-@#$%^`~&*+=/\'\"<>",
    *next = NULL,
    *l = strdup(line);

    while( (next = next_token(&l, delim)) != NULL)
    {
        LOG("token received from line being saved is %s\n", next);
        strcat(parsed_line, next);
    }

    
    free(l);
    return strdup(parsed_line);
}

/**
 * @param word
 * @brief
 */
bool case_sensitivity_matching(char * word)
{
    int i;
    for (i = 0; i < g_num_terms; i++)
    {
        LOG("Search term: '%s'; word: %s\n", g_search_terms[i], word);
        if(g_exact)
        {
            if(strcmp(word, g_search_terms[i]) == 0) return true;
        }
        else
        {
           if(strcasecmp(word, g_search_terms[i]) == 0) return true;
        }
    }

    return false;
}

/**
 * @param absolute_path
 * @param entry
 * @brief
 */
char * permission_decyper(char * absolute_path, struct dirent * entry)
{
    struct stat entry_info;
    char permission[4] = {'-','-','-','-'};

    if(stat(absolute_path, &entry_info) != 0) 
    {
        perror("stat() error");
        return strdup(permission);
    }
    else
    {
        if(entry_info.st_uid == getuid())
        {
            permission[0] = 'o';
            permission[1] = (entry_info.st_mode & S_IRUSR)? 'r' : '-';
            permission[2] = (entry_info.st_mode & S_IWUSR)? 'w' : '-';
            permission[3] = (entry_info.st_mode & S_IXUSR)? 'x' : '-';
        }
        else if (entry_info.st_gid == getgid())
        {
            permission[1] = (entry_info.st_mode & S_IRGRP)? 'r' : '-';
            permission[2] = (entry_info.st_mode & S_IWGRP)? 'w' : '-';
            permission[3] = (entry_info.st_mode & S_IXGRP)? 'x' : '-';
        }
        else
        {
            permission[1] = (entry_info.st_mode & S_IROTH)? 'r' : '-';
            permission[2] = (entry_info.st_mode & S_IWOTH)? 'w' : '-';
            permission[3] = (entry_info.st_mode & S_IXOTH)? 'x' : '-';
        }

        LOG("permissions found [%s]\n", permission);
        return strdup(permission);
    }
}

/**
 * @param absolute_path
 * @param line_number
 * @param permissions
 * @param line
 * @brief
 */
void print_line_diagnostics(char * absolute_path, int line_number, char * permissions, char * line)
{
    printf("[ %s | %d | %s | %s ]\n", absolute_path, line_number, permissions, line);
}

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
char * next_token(char **str_ptr, const char *delim)
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

