#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <assert.h>

#include "debug.h"
#include "history.h"
#include "timer.h"
#include "tokenizer.h"
#include "user_info.h"
#include "built_in.h"
#include "pipe.h"

int count = 0;
int hist_tracker = 0;
bool found = false;
bool scripting = false;
char short_path[PATH_MAX];
char current_dir[PATH_MAX];
bool history_check = false;
bool double_ex = false;

void sigint_handler(int signo) {
	printf("It worked\n");
    exit(0);
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

void check_path() {
	char full_path[PATH_MAX];
	memset(full_path, 0, PATH_MAX);
	strcpy(full_path, getcwd(current_dir, sizeof(current_dir)));

	DIR *dir = opendir(getcwd(current_dir, sizeof(current_dir)));

	if (dir != NULL) {
		if (strncmp(full_path, HOME, strlen(HOME)) == 0) {
			found = true;
			memset(short_path, 0, PATH_MAX);
			parse_home(full_path);
		}
	}
	closedir(dir);
}


void print_prompt() {
	found = false;
	check_path();

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

	signal(SIGINT, sigint_handler);

	while(true) {
		if (isatty(STDIN_FILENO)) {
			scripting = false;
    	} else {
			getcwd(current_dir, sizeof(current_dir));
			scripting = true;
    	}


    	if (!scripting) {
			print_prompt();
    	}

		char *line = NULL;
		size_t line_sz = 0;
		int line_ptr = getline(&line, &line_sz, stdin);
		if (line_ptr == -1) {
			free(line);
			break;
		}

		char *line_dupicate = line;
		int total = 0;
		char *ptr = line_dupicate;
		while((ptr = strchr(ptr, ' ')) != NULL) {
    		total++;

    		if (total >= _POSIX_ARG_MAX) {
    			break;
    		}
    		ptr++;
		}

		if (total >= _POSIX_ARG_MAX) {
			break;
		}

		if (strcmp(line, "\n") != 0) {
			count++;
			if (strcmp(line, "!!\n") == 0 && hist_tracker >= 1) {
				double_ex = true;
			}

			if (double_ex == true) {
				int index = hist_tracker - 1;
				if (index >= HIST_MAX) {
					index = hist_tracker % HIST_MAX;
				}
				
				strcpy(line, double_exclamation(index));
				add_history(line, hist_tracker);
				hist_tracker++;
				
			} else {
				if (starts_with(line)) {
					if (is_numeric(line)) {	
						if (num_search(line, hist_tracker) == NULL) {
							continue;
						} else {
							strcpy(line, num_search(line, hist_tracker));
						}
					} else {
						if (prefix_search(line, hist_tracker) == NULL) {
							continue;
						} else {
							strcpy(line, prefix_search(line, hist_tracker));
						}
					}
				} else if (strcmp(line, "history\n") == 0) {
					add_history(line, hist_tracker);
					hist_tracker++;
					history_check = true;
				} else {
					add_history(line, hist_tracker);
					hist_tracker++;
				}

				if (history_check) {
					print_history(hist_tracker);
					history_check = false;
					continue;
				}
			}
		}

		comment_check(line);
		if (COMMENTS) {
			strcpy(line, NEW_LINE);
			strcat(line, "\n");
		}

		double_ex = false;
		char *tokens[_POSIX_ARG_MAX];
		int i = 0;
		int total_tokens = 0;
		char *next_tok = line;
		char *curr_tok;
		while ((curr_tok = next_token(&next_tok, " \t\r\n", total_tokens)) != NULL) {
			if (total_tokens < _POSIX_ARG_MAX) {
				tokens[i++] = curr_tok;
				total_tokens++;
			} else {
				break;
			}
		}

		if (total_tokens >= _POSIX_ARG_MAX) {
			break;
		}

		piping = false;
		int pipe_counter = 0;
		struct command_line cmds[100];
		// char *command1[] = {"ls", "-1", "/", (char *) NULL};

		// command1[0] = tokens[0];
		// command1[1] = "this is mhshahid hello world";
		// command1[3] = (char *) NULL;

		// cmds[0].tokens = command1;
		// cmds[0].stdout_pipe = false;
		// cmds[0].stdout_file = NULL;

		memset(new_token, 0, sizeof(new_token));
		num_commands = 0;

		for (int i = 0; i < total_tokens; i++) {
			char temp_tok[_POSIX_ARG_MAX];
			memset(temp_tok, 0, _POSIX_ARG_MAX);
			strcpy(temp_tok, tokens[i]);

			char tok[_POSIX_ARG_MAX];
			memset(tok, 0, _POSIX_ARG_MAX);

			if (strstr(temp_tok, "$")) {
				strcpy(tok, env_check(temp_tok));
				if (strcmp(tok, "") == 0) {
					strcpy(tok, temp_tok);
				}
			} else {
				strcpy(tok, temp_tok);
			}

			add_token(tok, num_commands);
			num_commands++;
		}

		populate_struct(cmds, num_commands);

		memset(tokens, 0, sizeof(tokens));
		int counter = 0;
		for (int i = 0; i < num_commands; i++) {
	        tokens[counter] = new_token[i];
	        counter++;
	    }

		free(line);

		if (total_tokens != 0) {
			if (strcmp(tokens[0], "cd") == 0) {
				if (total_tokens == 2) {
					change_directory(tokens[1]);
					memset(current_dir, 0, PATH_MAX);
					getcwd(current_dir, sizeof(current_dir));
				} else if (total_tokens == 1) {
					memset(current_dir, 0, PATH_MAX);
					chdir(HOME);
					getcwd(current_dir, sizeof(current_dir));
				} else if (total_tokens > 2) {
					printf("-cash: cd: %s: No such file or directory\n", tokens[1]);
				}
			} else if (strcmp(tokens[0],"setenv") == 0) {
				if (total_tokens == 3) {
					set_env(tokens[1], tokens[2]);
				}
			} else if (strcmp(tokens[0], "jobs") == 0 && total_tokens == 1) {

			} else if (strcmp(tokens[0], "exit") == 0) {
				break;
			} else {
				tokens[i] = (char *) NULL;
				pid_t pid = fork();
				if (pid == 0) {
					/* child */
					// if (piping) {
					// 	printf("here\n");
					// 	execute_pipeline(cmds);
					// } else {
						int ret = execvp(tokens[0], tokens);
						close(STDIN_FILENO);
						if (ret == -1) {
							break;
						}
					// }
				} else if (pid == -1) {
					perror("fork");
				} else {
					/* parent */
					int status;
					waitpid(pid, &status, 0);
				}
			}
		}
		piping = false;

	}

	return 0;
}
