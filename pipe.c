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

void populate_struct(struct command_line *cmds, int num_commands) {
	printf("Printing populate struct\n");

	int counter = 0;
	int start = 0;
	int end = 0;
	bool contains = false;

	for (int i = 0; i < num_commands; i++) {
		char *parsed_tok = new_token[i];
		
		if (strcmp(parsed_tok, "|") == 0 || (i + 1 >= num_commands)) {
			char *command[_POSIX_ARG_MAX];
			int track = 0;
			printf("Conuter: %d\n", counter);

			if ((i + 1) >= num_commands) {
				contains = false;
				end = i + 1;
			} else if (strcmp(parsed_tok, "|") == 0) {
				contains = true;
				end = i;
			}

			for (int j = start; j < end; j++) {
				command[track] = new_token[j];
				track++;
			}

			command[track] = (char *) NULL;
			printf("Printing command:\n");
			for (int k = 0; k < track; k++) {
				char *position = command[k];
				while (*position != '\0') {
					printf("%c", *(position++));
				}
				strcpy(position, "");
				printf("\n");
			}
			cmds[counter].tokens = command;
			printf("%s      %d\n", *command, counter);
			cmds[counter].stdout_file = NULL;
			if (contains) {
				cmds[counter].stdout_pipe = true;
			} else {
				// printf("This is cmds->tokens[1]: %s\n", cmds[counter].tokens[1]);
				cmds[counter].stdout_pipe = false;
			}

			if ((end + 1) < num_commands) {
				start = end + 1;
			} else {
				counter++;
				break;
			}

			i = end;
			counter++;
		}
	}


	printf("Finally executing:\n");	
	printf("%s\n", cmds[0].tokens[0]);
	execute_pipeline(cmds);
}

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
		printf("Last Command: %s\n", cmds->tokens[0]);
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
					printf("Command: %s\n", cmds->tokens[0]);
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
					execute_pipeline(cmds + 1);
				}
			}
		}
	}
}