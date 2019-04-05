#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

#include <limits.h>
#include <stdbool.h>

char search_result[_POSIX_ARG_MAX];
char history[HIST_MAX][100];

struct history_entry {
    unsigned long cmd_id;
    double run_time;
    // char command[ARG_MAX]
    /* What else do we need here? */
};

bool starts_with(char *token);
bool is_numeric(char *line);
void append(char *s, char c);
int smallest_index();
void add_history(char *line, int hist_tracker);
void print_history(int hist_tracker);
char *double_exclamation(int index);
char *prefix_search(char *prefix, int hist_tracker);
char *num_search(char *num, int hist_tracker);

#endif
