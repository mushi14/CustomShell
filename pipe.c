#include "pipe.h"
#include "tokenizer.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>

/**
 * Function that parses the tokens into commands and stores it into the cmds struct.
 * The values are read from the new_token array, an array of all tokens parsed, cleaned,
 * and separated in order.
 * Param: cmds - command struct to populate for the specific tokens
 * Param: num_command - total number of tokens in the new_token array
 */
void populate_struct(struct command_line *cmds, int num_commands) {
	int counter = 0;
	int start = 0;
	int end = 0;
	bool contains = false;
	bool output = false;
	char *output_file;

	for (int i = 0; i < num_commands; i++) {
		char *parsed_tok = new_token1[i];

		if (strcmp(parsed_tok, "|") == 0 || strcmp(parsed_tok, ">") == 0 || (i + 1 >= num_commands)) {
			cmds[counter].tokens = malloc(num_commands * sizeof(char *));
			int track = 0;

			if ((i + 1) >= num_commands) {
				contains = false;
				end = i + 1;
			} else if (strcmp(parsed_tok, "|") == 0) {
				contains = true;
				end = i;
			} else if (strcmp(parsed_tok, ">") == 0) {
				output = true;
				contains = false;
				end = i;
			}

			for (int j = start; j < end; j++) {
				cmds[counter].tokens[track] = new_token1[j];
				track++;
			}
			cmds[counter].tokens[track] = (char *) NULL;

			if (contains) {
				cmds[counter].stdout_pipe = true;
			} else {
				cmds[counter].stdout_pipe = false;
			}

			if (output) {
				start = i + 1;
				end = start + 1;
				for (int j = start; j < end; j++) {
					output_file = new_token1[j];
				}

				cmds[counter].stdout_file = output_file;
				counter++;
				break;
			} else {
				cmds[counter].stdout_file = NULL;
			}

			if ((end + 1) < num_commands) {
				start = end + 1;
			}

			i = end;
			counter++;
		}
	}

	pid_t pid = fork();
	if (pid == 0) {
		execute_pipeline(cmds);
	} else if(pid == -1) {
		perror("pid error");
		return;
	} else {
		int status;
		waitpid(pid, &status, 0);
	}

	for (int i = 0; i < counter; i++) {
		free(cmds[i].tokens);
	}

	output_file = NULL;
}

/**
 * Recursive function that executes the given struct of commands and stores to either an output
 * file or prints to the terminal, depending on if given an output file.
 * Param: cmds - command struct to populate for the specific tokens
 */
void execute_pipeline(struct command_line *cmds) {
	if(cmds->stdout_pipe == false) {
		if(cmds->stdout_file != NULL) {
			int open_flags = O_RDWR | O_CREAT | O_TRUNC;
			int fd = open(cmds->stdout_file, open_flags, 0666);
			if(fd == -1) {
				perror("couldn't open file");
				return;
			} else {
				if(dup2(fd, STDOUT_FILENO) == -1) {
					perror("error");
					return;
				}
			}
		}

		int result = 0;
		result = execvp(cmds->tokens[0], cmds->tokens);
		if(result == -1) {
			perror("execvp error");
		}

		return;
	} 

	if(cmds->stdout_pipe == true) {
		int fd[2];
		if(pipe(fd) == -1) {
			perror("couldn't open file");
			return;
		} else {
			pid_t pid = fork();

			if(pid == 0) {
				if(dup2(fd[1], STDOUT_FILENO) == -1) {
					perror("error");
					return;
				} else {
					close(fd[0]);
					int value = execvp(cmds->tokens[0], cmds->tokens);
					if(value == -1) {
						perror("execvp");
						return;
					}
				}
			} else if(pid == -1) {
				perror("pid error");
				return;
			} else {
				if(dup2(fd[0], STDIN_FILENO) == -1) {
					perror("error");
				} else {
					close(fd[1]);
					execute_pipeline(cmds+1);
				}
			}
		}
	}
}