/**
 * @file
 * history.h
 */
#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100
#define COMMAND_SIZE 4096
/**
 * @brief history entry
 */
struct history_entry {
    unsigned int cmd_id;
    /* What else do we need here? Note: you aren't required to use this struct,
     * it is only here for demonstration purposes. */
    char command[COMMAND_SIZE];
};

/**
 * @brief history container
 */
struct history_container {
	int entry_count;
	int head;
	int tail;
	int min;
	struct history_entry list[HIST_MAX];
};

/**
 * @param cmd cmd in index
 */
void run_command_id(int cmd);

/**
 * @param prefix char prefix to check in history from the head to the tail
 */
int run_prefix_command(char * prefix);

/**
 * @param void !!
 */
void run_last_history_command();

/**
 * @param id command id
 */
struct history_entry * get_entry(unsigned int id);

/**
 * @param id_idx set index
 */
void set_entry(int id_idx);

/**
 * @param id find command
 */
int find_entry_idx(unsigned int id);

/**
 * @param void initialization
 */
void create_history_log();

/**
 * @param void print history
 */
void print_history();

/**
 * @param void 
 */
void shift_entries();

/**
 * @param command_number command number
 * @param shell command
 */
void add_entry(unsigned int command_number, char * user_command);

#endif
