#include <stdbool.h>

struct command_line {
	char **tokens;
	bool stdout_pipe;
	char *stdout_file;
};

bool piping;
void populate_struct(struct command_line *cmds, int num_commands);
void execute_pipeline(struct command_line *cmds);
