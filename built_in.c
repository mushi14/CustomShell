#include "built_in.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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