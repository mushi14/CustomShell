#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <stdbool.h>
#include <limits.h>
/* 
 * Takes in a line and tokenizes it by the given delimiter. 
 */
// bool is_env = false;
bool token_seen;
char tok_ret[_POSIX_ARG_MAX];
int num_commands;
char new_token[_POSIX_ARG_MAX][_POSIX_ARG_MAX];
char new_token1[_POSIX_ARG_MAX][_POSIX_ARG_MAX];

char *next_token(char **str_ptr, const char *delim, int total_tokens);
char *env_check(char *tokens);
char *expand_var(char *str);
void add_token(char *line, int index);
void print_token(int index);

#endif
