/**
 * @file prompt.c
 * @brief prompt that prints the shell
 */
#include <limits.h>
#include "prompt.h"
#include "string_functions.h"

/*global from prompt.h*/
#define CODES 8
unsigned int CMD = 0;
bool newCode = false;
char hostname[HOST_NAME_MAX] = {0};
char * login = NULL;
const char * tilda = "~";

/*program globals*/
//char shades[] = { 0xf0, 0x9f, 0x98, 0x8e, 0};
//char hands[] = { 0xE2, 0x9C, 0x8B, 0xF0, 0x9F, 0x8F, 0xBE};
char codes[][CODES] = {
	{ 0xf0, 0x9f, 0x98, 0x8e, 0},
	{ 0xE2, 0x9C, 0x8B, 0xF0, 0x9F, 0x8F, 0xBE}
};

/**
 * @param shellname its your shellname
 * @param exitcode error code
 */
void print_shell_prompt(char * shellname, int exitcode)
{
    if(strcmp(shellname, "shellname") == 0) shellname = get_shell_name();
    if(newExitCode()) setExitCode(true);
    gethostname(hostname, HOST_NAME_MAX);
    login = getlogin();
    printf("\n[%s]%s[%u]%s@%s: ", codes[exitcode], shellname, CMD, login, hostname);
    return;
}
/**
 * @param nc new code
 */
void setExitCode(bool nc)
{
	newCode = nc;	
}

/**
 * @param void new exit code?
 */
bool newExitCode()
{
	return (newCode == true)? true : false;
}

/**
 * @param void incremenet command number
 */
void increment_cmd() { CMD++; }

/**
 * @param void command number
 */
unsigned int get_cmd() { return CMD; }
