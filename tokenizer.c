#include "tokenizer.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#include "history.h"

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
char *next_token(char **str_ptr, const char *delim, int total_tokens) {
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    char quotes[_POSIX_ARG_MAX] = "\'\"";
    size_t quote_start = strcspn(*str_ptr, quotes);
    int extra = 0;
    
    if (quote_start < tok_end) {
        char temp[_POSIX_ARG_MAX];
        memset(temp, 0, _POSIX_ARG_MAX);
        strcpy(temp, *str_ptr);
        
        size_t len = strlen(temp);
        if (len > 0 && temp[len-1] == '\n') {
            temp[--len] = '\0';
        }

        quotes[0] = *(*str_ptr + quote_start);
        quotes[1] = '\0';

        size_t offset = quote_start + 1;
        size_t quote_end = strcspn(*str_ptr + offset, quotes) + offset;
        tok_end = strcspn(*str_ptr + quote_end, delim) + quote_end - tok_start;

        if (strlen(temp) - quote_end > 1) {
            extra = 2;
        } else {
            extra = 1;
        }

        memmove(&temp[quote_start], &temp[quote_start + 1], strlen(temp) - quote_start);
        memmove(&temp[quote_end - 1], &temp[quote_end], strlen(temp) - quote_end + extra);

        *str_ptr = temp;
    }

    /* Zero length token. We must be finished. */
    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end - extra;

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

char *env_check(char *token) {
    token_seen = false;
    char *tok_arr[_POSIX_ARG_MAX];
    int i = 0;

    char tok_copy[_POSIX_ARG_MAX];
    memset(tok_ret, 0, _POSIX_ARG_MAX);
    memset(tok_copy, 0, _POSIX_ARG_MAX);
    strcpy(tok_copy, token);


    if (strstr(token, " ")) {
        // printf("multiple words in token\n");
        if (strstr(token, "$")) {
            int counter = 0;
            char *tok = strtok(tok_copy, " \t\r\n");
            while (tok != NULL) {
                bool seen = false;
                counter++;
                int start = strcspn(tok, "$");
                if (start == 0) {
                    char *parsed = expand_var(tok);
                    if (parsed != NULL) {
                        token_seen = true;
                        seen = true;
                        tok_arr[i] = parsed;
                        i++;
                    }
                }
                if (!seen) {
                    tok_arr[i] = tok;
                    i++;
                }
                tok = strtok(NULL, " \t\r\n");
            }
        }
    } else {
        int start = strcspn(token, "$");
        if (start == 0) {
            char *parsed = expand_var(tok_copy);
            if (parsed != NULL) {
                token_seen = true;
                char replace[_POSIX_ARG_MAX];
                memset(replace, 0, _POSIX_ARG_MAX);
                strcpy(tok_ret, parsed);
            }
        }
    }

    for (int j = 0; j < i; j++) {
        char *holder = tok_arr[j];
        strcat(tok_ret, holder);
        if ((j + 1) < i) {
            strcat(tok_ret, " ");
        }
    }

    return tok_ret;
}

char *expand_var(char *str) {
    size_t var_start = 0;
    var_start = strcspn(str, "$");
    if (var_start == strlen(str)) {
        /* No variable to replace */
        return NULL;
    }

    size_t var_len = strcspn(str + var_start, " \t\r\n\'\"");

    char *var_name = malloc(sizeof(char) * var_len + 1);
    if (var_name == NULL) {
        return NULL;
    }
    strncpy(var_name, str + var_start, var_len);
    var_name[var_len] = '\0';

    if (strlen(var_name) <= 1) {
        free(var_name);
        return NULL;
    }

    /* Use index 1 to ignore the '$' prefix */
    char *value = getenv(&var_name[1]);
    if (value == NULL) {
        value = "";
    }

    free(var_name);

    /* Grab the size of the remaining string (after the $var): */
    size_t remain_sz = strlen(str + var_start + var_len);

    /* Our final string contains the prefix data, the value of the variable, the
     * remaining string size, and an extra character for the NUL byte. */
    size_t newstr_sz = var_start + strlen(value) + remain_sz + 1;

    char *newstr = malloc(sizeof(char) * newstr_sz);
    if (newstr == NULL) {
        return NULL;
    }

    strncpy(newstr, str, var_start);
    newstr[var_start] = '\0';
    strcat(newstr, value);
    strcat(newstr, str + var_start + var_len);

    return newstr;
}

void add_token(char *line, int index) {
    strcat(new_token[index], line);
}

void print_token(int index) {
    for (int i = 0; i < index; i++) {
        char *position = new_token[i];

        while (*position != '\0') {
            printf("%c", *(position++));
        }
        strcpy(position, "");

        printf("\n");
    }
}
