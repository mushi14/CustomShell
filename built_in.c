#include "built_in.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int is_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

char *path_converter(char *path, char *name) {
	char *new_name = name;
	char *temp = "/";
	char *new_dir = (char *) malloc(100 + strlen(path) + strlen(new_name));
	char *a = (char *) malloc(100 + strlen(path) + strlen(temp));
	strcpy(a, path);
	strcat(a, temp);
	strcpy(new_dir, a);
	strcat(new_dir, new_name);

	return new_dir;
}

void change_directory(char *target, bool path) {
	char cwd[PATH_MAX];
	memset(cwd, 0, PATH_MAX);
	getcwd(cwd, sizeof(cwd));

	DIR *dir;
	struct dirent *sub_dir;
	bool check = false;
	int tracker = 1;

	if (path == true) {
		dir = opendir(target);
		if (dir != NULL) {
			tracker = 0;
			chdir(target);
			getcwd(cwd, sizeof(cwd));
		}
	} else {
		dir = opendir(cwd);
		if (dir != NULL) {
			while ((sub_dir = readdir(dir)) != NULL) {
				if (strcmp(sub_dir->d_name, ".") != 0 && strcmp(sub_dir->d_name, "..") != 0) {
					char *new_dir = path_converter(cwd, sub_dir->d_name);
					if (is_file(new_dir) == 0 && strcmp(sub_dir->d_name, target) == 0) {
						check = true;
						tracker = 0;
						chdir(sub_dir->d_name);
						getcwd(cwd, sizeof(cwd));
						free(new_dir);
						break;
					}
					free(new_dir);
				}
			}
		}
	}
	closedir(dir);

	if (!check && tracker > 0) {
		printf("-cash: cd: %s: No such file or directory\n", target);
	}
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