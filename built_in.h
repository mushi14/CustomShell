#define BUF _POSIX_ARG_MAX

#include <stdbool.h>
#include <limits.h>

char NEW_LINE[BUF];
bool COMMENTS;
char ret[10000];

int is_file(const char *path);
char *path_converter(char *path, char *name);
void change_directory(char *target);
void comment_check(char *line);
void set_env(char *name, char*value);