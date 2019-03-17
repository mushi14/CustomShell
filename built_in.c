#include "built_in.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void comment_check(char *line) {
	if (strstr(line, "#")) {
		int end_line = strcspn(line, "#");
		for (int i = 0; i < end_line; i++) {
			NEW_LINE[i] = line[i];
		}
	}
}