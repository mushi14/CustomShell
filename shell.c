#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#include "debug.h"
#include "history.h"
#include "timer.h"
#include "tokenizer.h"
#include "user_info.h"
#include "built_in.h"

#define ANSI_COLOR_RED "\x1b[31m"

int count = 0;
bool found = false;
bool call_cd = false;
char short_path[PATH_MAX];
char current_dir[PATH_MAX];

int is_file(const char *path) {
	struct stat path_stat;
	stat(path, &path_stat);

	return S_ISREG(path_stat.st_mode);
}

char *path_converter(char *path, char *name, bool slash) {
	char *new_name = name;
	char *temp;
	if (slash) {
		temp = "/";
	} else {
		temp = "";
	}
	char *new_dir = (char *) malloc(100 + strlen(path) + strlen(new_name));
	char *a = (char *) malloc(100 + strlen(path) + strlen(temp));
	strcpy(a, path);
	strcat(a, temp);
	strcpy(new_dir, a);
	strcat(new_dir, new_name);

	return new_dir;
}

void parse_home(char *line) {
	if (strstr(line, USERNAME)) {
		int size = strlen(line);
		int temp = size - strlen(strstr(line, USERNAME));
		int start = strcspn(strstr(line, USERNAME), "/") + temp;

		for (int i = start; i < size; i++) {
			int len = strlen(short_path);
			short_path[len] = line[i];
			short_path[len + 1] = '\0';
		}
	}
}

void *change_directory(char *target, bool path) {
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
					char *new_dir = path_converter(cwd, sub_dir->d_name, true);
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

void traverse(char *home) {
	DIR *dir = opendir(home);
	struct dirent *sub_directory;
	char name[200];
	memset(name, 0, 200);
	strcpy(name, home);

	if (dir != NULL) {
		while ((sub_directory = readdir(dir)) != NULL) {
			char *new_dir = path_converter(name, sub_directory->d_name, true);

			if (strcmp(sub_directory->d_name, ".") != 0 && strcmp(sub_directory->d_name, "..") != 0) {
				if (strcmp(getcwd(current_dir, sizeof(current_dir)), new_dir) == 0) {
					found = true;
					memset(short_path, 0, PATH_MAX);
					parse_home(new_dir);
					break;
				} else if (is_file(new_dir) == 0) {
					traverse(new_dir);
				}
			}
			free(new_dir);
		}
	}
	closedir(dir);
}

int print_prompt() {
	found = false;
	traverse(HOME);

	if (found) {
		printf("[%d %s@%s:~%s]$ ", count, USERNAME, HOSTNAME, short_path);
	} else {
		getcwd(current_dir, sizeof(current_dir));
		if (strcmp(current_dir, HOME) == 0) {
			printf("[%d %s@%s:~]$ ", count, USERNAME, HOSTNAME);
		} else {
			printf("[%d %s@%s:%s]$ ", count, USERNAME, HOSTNAME, current_dir);
		}
	}
	fflush(stdout);
}

int main(void) {

    /*double start = get_time();
    print_history();
    sleep(1);
    double end = get_time();

    printf("Time elapsed: %fs\n", end - start);*/

	get_user();
	get_hostname();
	get_home_dir();

	while(true) {
		print_prompt();
		char *line = NULL;
		size_t line_sz = 0;
		getline(&line, &line_sz, stdin);

		// comment_check(line);
		// if (COMMENTS) {
		// 	strcpy(line, NEW_LINE);
		// 	strcat(line, "\n");
		// }

		if (strcmp(line, "\n") != 0) {
			count++;
		}

		char *tokens[1000];
		int i = 0;
		int total_tokens = 0;
		char *next_tok = line;
		char *curr_tok;
		while ((curr_tok = next_token(&next_tok, " \t\r\n")) != NULL) {
			tokens[i++] = curr_tok;
			total_tokens++;
		}

		bool path = false;
		// for (int i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++) {
		// 	printf("%s\n", tokens[i]);
		// }
		if (total_tokens <= 2 && strcmp("cd", tokens[0]) == 0) {
			if (total_tokens > 1) {
				if (strstr(tokens[1], "/")) {
					path = true;
				}
			}

			if (total_tokens == 2) {
				change_directory(tokens[1], path);
				memset(current_dir, 0, PATH_MAX);
				getcwd(current_dir, sizeof(current_dir));
			} else if (total_tokens == 1) {
				memset(current_dir, 0, PATH_MAX);
				chdir(HOME);
				getcwd(current_dir, sizeof(current_dir));
			}
		} else if (total_tokens > 2 && strcmp("cd", tokens[0]) == 0) {
			printf("-cash: cd: %s: No such file or directory\n", tokens[1]);
		} else if (total_tokens != 0) {
			tokens[i] = (char *) NULL;

			pid_t pid = fork();
			if (pid == 0) {
				/* child */
				int ret = execvp(tokens[0], tokens);
				if (ret == -1) {
					printf("-cash: %s: command not found\n", line);
				}
			} else if (pid == -1) {
				perror("fork");
			} else {
				/* parent */
				int status;
				waitpid(pid, &status, 0);
				LOG("Child exited. Status: %d\n", status);
			}
		}
	}

	return 0;
}
