#include "built_in.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

/**
 * Checks to see if the given path is a file or not
 * Param: path - fie to analyze
 * Return: 0 if it is regular file or 1 if it is a directory
 */
int is_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

/**
 * Changes the current directory to the given target directory
 * Param: target - target directory to change the current working 
 *        directory to
 */
void change_directory(char *target) {
	char cwd[PATH_MAX];
	memset(cwd, 0, PATH_MAX);
	getcwd(cwd, sizeof(cwd));

	DIR *dir;
	dir = opendir(target);

	if (dir != NULL) {
		int ret = chdir(target);
		if (ret != -1) {
			getcwd(cwd, sizeof(cwd));
		}
	}
	closedir(dir);
}

/**
 * Checks to see the given line contains any comments
 * Param: line - line to check
 */
void comment_check(char *line) {
	COMMENTS = false;
	memset(NEW_LINE, 0, BUF);
	if (strstr(line, "#")) {
		COMMENTS = true;
		int end_line = strcspn(line, "#");
		for (int i = 0; i < end_line; i++) {
			NEW_LINE[i] = line[i];
		}
	}
}

/**
 * Sets up a custom environment variable to the given name and value
 * Param: name - name for the custom environment variable
 * Param: value - value for the custom environment variable
 */
void set_env(char *name, char *value) {
	int ret = setenv(name, value, 0);
	if (ret == -1) {
		printf("Cannot set environment variable. \n");
	}
}