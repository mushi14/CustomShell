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

int is_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

char *path_converter(char *path, char *name) {
	memset(ret, 0, 10000);

	char *new_name = name;
	char *temp = "/";
	char *new_dir = (char *) malloc(100 + strlen(path) + strlen(new_name));
	char *a = (char *) malloc(100 + strlen(path) + strlen(temp));
	strcpy(a, path);
	strcat(a, temp);
	strcpy(new_dir, a);
	strcat(new_dir, new_name);
	strcpy(ret, new_dir);

	free(a);
	free(new_dir);
	return ret;
}

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

void set_env(char *name, char *value) {
	int ret = setenv(name, value, 0);
	if (ret == -1) {
		printf("Cannot set environment variable. \n");
	}
}