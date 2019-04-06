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
bool redirection = false;
bool jobs = false;
int carry_on = 0;

/**
 * Checks if the background jobs are done. Sets as false, if so.
 */
void sigchild_handler(int signo) {
	jobs = false;
}

/**
 * Checks for ^C, exits the program gracefully if so.
 */
void sigint_handler(int signo) {
	printf("It worked\n");
	exit(0);
}

/**
 * Parses the home directory to find the name of the sub-directory. 
 * Needed for printing the prompt
 * Param: line - path to parse
 */
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

/**
 * Checks to see if the path is in the home directory
 */
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

/**
 * Prints the prompt
 */
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
	// Setting username, hostname, and homedirectory
	get_user();
	get_hostname();
	get_home_dir();

	// Handling signals for sigin and sigchld
	if (jobs) {
		signal(SIGCHLD, sigchild_handler);
	} else {
		signal(SIGINT, sigint_handler);
	}

	while(true) {
		// Checking for scripting mode
		if (isatty(STDIN_FILENO)) {
			scripting = false;
    	} else {
			getcwd(current_dir, sizeof(current_dir));
			scripting = true;
    	}

    	// Printing prompt
    	if (!scripting) {
			print_prompt();
    	}

    	// Checking to see if the token size is valid
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
			free(line);
			break;
		}

		// Checking for piping
		if (strstr(line, "|")) {
			piping = true;
		}

		// Checking for redirection
		if (strstr(line, ">")) {
			redirection = true;
		} else {
			redirection = false;
		}

		// Just the basic background jobs
		int job_symbol = strcspn(line, "&");
		if (job_symbol == (strlen(line) - 2)) {
			jobs = true;
		}

		// History
		if (strcmp(line, "\n") != 0) {
			count++;
			// Checks if the line is double bang, which prints the last command
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
				// Checking if bang prefix or bang number of the command
				if (starts_with(line)) {
					if (is_numeric(line)) {	
						if (num_search(line, hist_tracker) == NULL) {
							free(line);
							continue;
						} else {
							strcpy(line, num_search(line, hist_tracker));
						}
					} else {
						if (prefix_search(line, hist_tracker) == NULL) {
							free(line);
							continue;
						} else {
							strcpy(line, prefix_search(line, hist_tracker));
						}
					}
				// Checks and prints the history 
				} else if (strcmp(line, "history\n") == 0) {
					add_history(line, hist_tracker);
					free(line);
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

		// Checking for comments
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
		// Parsing line into tokens
		while ((curr_tok = next_token(&next_tok, " \t\r\n", total_tokens)) != NULL) {
			if (total_tokens < _POSIX_ARG_MAX) {
				tokens[i++] = curr_tok;
				total_tokens++;
			} else {
				break;
			}
		}

		if (total_tokens >= _POSIX_ARG_MAX) {
			free(line);
			break;
		}

		memset(new_token, 0, sizeof(new_token));
		num_commands = 0;

		// Adds the tokens parsed, cleaned, and converted to values of env variables
		// into a new array
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

		memset(tokens, 0, sizeof(tokens));
		int counter = 0;
		int struct_size = 0;
		int ind = 0;
		char *out_file;

		// Sets the tokens back to the array with the cleaned, parsed version of all tokens
		for (int i = 0; i < num_commands; i++) {
			if (strcmp(new_token[i], ">") == 0) {
				ind = i + 1;
				break;
			}
			tokens[counter] = new_token[i];
			if (strcmp(tokens[counter], "|") == 0) {
				struct_size++;
			}
			counter++;
		}

		struct command_line cmds[struct_size + 1];
		memcpy(new_token1, new_token, sizeof(new_token));

		// Performs piping if necessary
		if (piping) {
			populate_struct(cmds, num_commands);
			continue;
		}

		free(line);

		if (total_tokens != 0) {
			// Performing cd
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
			// Setting env variables
			} else if (strcmp(tokens[0],"setenv") == 0) {
				if (total_tokens == 3) {
					set_env(tokens[1], tokens[2]);
				}
			// Exits
			} else if (strcmp(tokens[0], "exit") == 0) {
				break;
			} else {
				tokens[i] = (char *) NULL;
				// Forking
				pid_t pid = fork();
				if (pid == 0) {
					// Child
					if (!piping) {
						// Redirection
						if (redirection) {
							out_file = new_token[ind];
							if(out_file != NULL) {
								int open_flags = O_RDWR | O_CREAT | O_TRUNC;
								int fd = open(out_file, open_flags, 0666);
								if(fd == -1) {
									perror("couldn't open file");
								} else {
									if(dup2(fd, STDOUT_FILENO) == -1) {
										perror("error");
									}
								}
							}

							int ret = execvp(tokens[0], tokens);
							if (ret == -1) {
								break;
							}
						} else {
							int ret = execvp(tokens[0], tokens);
							close(STDIN_FILENO);
							if (ret == -1) {
								break;
							}
						}
					}
				} else if (pid == -1) {
					perror("fork");
				} else {
					// Parent
					int status;
					if (jobs) {
						waitpid(-1, &status, WNOHANG);
					} else {
						waitpid(pid, &status, 0);
					}
				}
			}
		}
	}

	return 0;
}
