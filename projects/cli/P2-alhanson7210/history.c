/**
 * @file history.c
 */

#include "history.h"
#include "string_functions.h"
#include "built_in_commands.h"
#include "debug.h"

#include <stdio.h>

struct history_container history_obj = { 0 };

/**
 * @param prefix running the prefix bang command
 */
int run_prefix_command(char * prefix)
{
	if(history_obj.entry_count == 0) return -1;
	LOGP("history has more than zero entries\n");
	if(history_obj.min == 0)
	{
		int i;
		for(i = history_obj.head; i >= history_obj.tail; i--)
		{
			if(strncmp(prefix, history_obj.list[i].command, strlen(prefix)) == 0)
			{
				LOG("found a matching command prefix %s\n", prefix);
				run_command(history_obj.list[i].command, " \t\n\r");
				return 0;
			}
		}
	}
	else
	{
		int j;
		for(j = history_obj.min-1; j >= history_obj.tail; j--)
		{
                        if(strncmp(prefix, history_obj.list[j].command, strlen(prefix)) == 0)
                        {
                                LOG("found a matching command prefix %s\n", prefix);
                                run_command(history_obj.list[j].command, " \t\n\r");
                                return 0;
                        }
		}
                for(j = history_obj.head; j >= history_obj.min; j--)
                {
                        if(strncmp(prefix, history_obj.list[j].command, strlen(prefix)) == 0)
                        {
                                LOG("found a matching command prefix %s\n", prefix);
                                run_command(history_obj.list[j].command, " \t\n\r");
                                return 0;
                        }
                }
	}
	LOGP("Prefix for bang command was not found\n");
	return -1;
}

/**
 * @param cmd print commmand 
 */
void run_command_id(int cmd)
{
    if(history_obj.entry_count == 0) return;
    if(history_obj.min != 0 && cmd >= history_obj.list[history_obj.min].cmd_id && cmd <= history_obj.list[history_obj.min-1].cmd_id)
    {
	LOGP("checking value range between min and min -1 circularly\n");
        int i = cmd % HIST_MAX;
        if(handle_built_in_command(history_obj.list[i].command) == -1)
        {
            LOGP("! command is not a built in\n");
            run_command(history_obj.list[i].command, " \t\n\r");
            return;
        }
	return;
    }
    
    LOGP("history has more than zero entries\n");
    int idx = cmd;
    int unmodded_h_cmd = (int) history_obj.list[history_obj.head].cmd_id;
    if(history_obj.min != 0) unmodded_h_cmd = (int) history_obj.list[history_obj.min-1].cmd_id;
    LOG("unmodded %d and command %d\n", unmodded_h_cmd, cmd);
    if(cmd < unmodded_h_cmd || cmd > unmodded_h_cmd)
    {
        LOGP("! command not in history entry\n");
        return;
    }
    LOGP("command is not less the smallest command in the history\n");
    if(cmd > HIST_MAX) idx = cmd % HIST_MAX;
    LOG("command idx trying to access in history %d\n", cmd);
    if(idx > HIST_MAX || idx < history_obj.min || idx < history_obj.tail)
    {
        LOGP("! command not in history entry\n");
        return;
    }
    LOGP("valid number within range\n");
    int h_cmd = (int) history_obj.list[idx].cmd_id % HIST_MAX;
    LOG("h_cmh is %d\n", h_cmd);
    if(h_cmd != idx)
    {
        LOGP("! command not in history entry\n");
        return;
    }
    LOG("h_cmd(%d) and cmd(%d) match\n", h_cmd, idx);
    LOGP("trying to run the command if it is a built in\n");
    if(handle_built_in_command(history_obj.list[idx].command) == -1)
    {
	LOGP("! command is not a built in\n");
        run_command(history_obj.list[idx].command, " \t\n\r");
    }
    LOG("leaving the loop for command %cmd\n", idx);
    return;
}

/**
 * @param void printing the history
 */
void run_last_history_command()
{
    if(history_obj.entry_count == 0) return;

    else
    {
        if(handle_built_in_command(history_obj.list[history_obj.head].command) == -1)
        {
            LOGP("! command is not a built in\n");
            run_command(history_obj.list[history_obj.head].command, " \t\n\r");
        }
    }
}

/**
 * @param void print history
 */
void print_history()
{
    /* This function should print history entries */
    if(history_obj.entry_count == 0)
    {
		LOGP("Not implemented yet: history_container does not have any entries.\n");
    	return;
    }
    /*default case where the history is printed from the tail to the head of the array of history commands*/
    if(history_obj.min == history_obj.tail) 
    {
		int i;
		for(i = history_obj.tail; i <= history_obj.head; i++)
			printf("%u %s\n", history_obj.list[i].cmd_id, history_obj.list[i].command);
    }
    else
    {
    	int j;
    	//print from the min val to the head to preserve order
    	for(j = history_obj.min; j <= history_obj.head; j++)
    		printf("%u %s\n", history_obj.list[j].cmd_id, history_obj.list[j].command);
    	//print from 
    	for(j = history_obj.tail; j < history_obj.min; j++)
    		printf("%u %s\n", history_obj.list[j].cmd_id, history_obj.list[j].command);
    }
}

/*
struct history_entry {
    unsigned int cmd_id;
    // What else do we need here? Note: you aren't required to use this struct,
     // it is only here for demonstration purposes.
    char command[COMMAND_SIZE];
};

struct history_container {
	int entry_count;
	int head;
        int min;
	int tail;
	struct history_entry list[HIST_MAX];
};
*/

/**
 * @param id command id
 */
struct history_entry * get_entry(unsigned int id)
{
	int i;
	for(i = 0; i < history_obj.entry_count; i++)
		if(history_obj.list[i].cmd_id == id) return &history_obj.list[i];
	
	fprintf(stderr,"No entry int the history container under the id %u", id);
	return NULL;
}

/**
 * @param id_idx set index in history to a specific value
 */
void set_entry(int id_idx)
{
	LOG("%d", id_idx);
	return;
}

/**
 * @param id command id
 */
int find_entry_idx(unsigned int id)
{
	return (int) id % HIST_MAX;
}

/**
 * @param void create history
 */
void create_history_log()
{
	history_obj.entry_count = 0;
	history_obj.head = -1;
	history_obj.tail = 0;
	history_obj.min = 0;
	static struct history_entry l = { 0 };
	history_obj.list[0] = l;
}

/**
 * @param void shift entries based off of min
 */
void shift_entries()
{
	history_obj.min = (history_obj.min + 1) % HIST_MAX;
}

/**
 * @param command_number the id of the command
 * @param user_command get line command
 */
void add_entry(unsigned int command_number, char * user_command)
{
	if(history_obj.entry_count < HIST_MAX)
	{	struct history_entry new_entry = { 0 };
		history_obj.list[history_obj.entry_count] = new_entry;
		history_obj.list[history_obj.entry_count].cmd_id = command_number;
		strcpy(history_obj.list[history_obj.entry_count].command, user_command);
		history_obj.head++;
		history_obj.entry_count++;
		LOG("entry command %s\n", history_obj.list[history_obj.head].command);
	}
	else 
	{
		history_obj.list[history_obj.min].cmd_id = command_number;
		strcpy(history_obj.list[history_obj.min].command, user_command);
		LOG("entry command %s\n", history_obj.list[history_obj.min].command);
		shift_entries();
	}
}

