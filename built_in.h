#define BUF 100

#include <stdbool.h>

char NEW_LINE[BUF];
bool COMMENTS;

int is_file(const char *path);
char *path_converter(char *path, char *name);
void change_directory(char *target, bool path);
void comment_check(char *line);