#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <ctype.h>

bool starts_with(char *token) {
	bool ret = false;
	
	int start = strcspn(token, "!");
	if (start == 0) {
		ret = true;
	}

	return ret;
}

bool is_numeric(char *line) {
	size_t len = strlen(line);
	if (len > 0 && line[len - 1] == '\n') {
		line[--len] = '\0';
	}

	char line_arr[10000];
	memset(line_arr, 0, 10000);
	strcpy(line_arr, line);
	char command[10000];
	memset(command, 0, 10000);
	int end = strcspn(line, " ");
	bool digit = false;

	for (int i = 1; i < end; i++) {
		append(command, line_arr[i]);
	}

	for (int i = 0; command[i] != '\0'; i++) { 
		if (isdigit(command[i])) {
			digit = true;
		} else {
			digit = false;
		}
	}

	return digit;
}

void append(char *s, char c) {
	int len = strlen(s);
	s[len] = c;
	s[len+1] = '\0';
}

int smallest_index() {
	char temp_history[HIST_MAX][_POSIX_ARG_MAX];
	memcpy(temp_history, history, sizeof(history));

	char compare[1000];
	memset(compare, 0, 1000);
	int convert = INT_MAX;
	int smallest = 0;

	for (int i = 0; i < HIST_MAX; i++) {
		char *pos = temp_history[i];
		
		if (strcmp(temp_history[i], "") == 0) {
			break;
		} else {
			while (*pos != ' ') {
				append(compare, *(pos++));
			}
		}

		int temp = atoi(compare);
		if (temp < convert) {
			convert = atoi(compare);
			smallest = i;
		}
		memset(compare, 0, 1000);
		strcpy(pos, "");
	}

	return smallest;
}

void add_history(char *line, int hist_tracker) {
	int index = hist_tracker;

	if (index >= HIST_MAX) {
		index = hist_tracker % HIST_MAX;
	}

	sprintf(history[index], "%d ", hist_tracker);
	strcat(history[index], line);
}

void print_history(int hist_tracker) {
	int smallest = smallest_index();

	for (int i = smallest; i < HIST_MAX; i++) {
		char *position = history[i];

		if (strcmp(history[i], "") == 0) {
			break;
		} else {
			while (*position != '\0') {
				printf("%c", *(position++));
			}
		}
		strcpy(position, "");
	}

	for (int i = 0; i < smallest; i++) {
		char *position = history[i];

		if (strcmp(history[i], "") == 0) {
			break;
		} else {
			while (*position != '\0') {
				printf("%c", *(position++));
			}
		}
		strcpy(position, "");
	}
}

char *double_exclamation(int index) {
	char entry[_POSIX_ARG_MAX];
	char *ret;
	char temp[2];
	memset(entry, 0, _POSIX_ARG_MAX);
	memset(temp, 0, 2);

	strcpy(entry, history[index]);
	int start = strcspn(entry, " ");
	temp[0] = entry[start + 1];
	char *p = &temp[0];

	ret = strstr(entry, p);
	return ret;
}

char *prefix_search(char *prefix, int hist_tracker) {
	int highest = 0;
	int index = 0;
	bool multiple = false;
	char compare[1000];
	memset(compare, 0, 1000);

	char temp_history2[HIST_MAX][_POSIX_ARG_MAX];
	memcpy(temp_history2, history, sizeof(history));
	char entry[_POSIX_ARG_MAX];
	char letter[2];
	memset(entry, 0, _POSIX_ARG_MAX);
	memset(letter, 0, 2);
	strcpy(entry, prefix);
	int start = strcspn(entry, "!");
	letter[0] = entry[start + 1];

	char *p = &letter[0];
	p = strstr(prefix, p);
	char matches[_POSIX_ARG_MAX];
	memset(matches, 0, _POSIX_ARG_MAX);

	size_t len = strlen(p);
	if (len > 0 && p[len-1] == '\n') {
		p[--len] = '\0';
	}

	for (int i = 0; i < HIST_MAX; i++) {
		char *temp = temp_history2[i];
		if (strstr(temp, p)) {
			char sub_str[_POSIX_ARG_MAX];
			memset(sub_str, 0, _POSIX_ARG_MAX);
			while (*temp != ' ') {
				append(sub_str, *temp);
				++temp;
			}

			int temp_count = atoi(sub_str);
			if (temp_count > highest) {
				highest = temp_count;
				index = i;
				multiple = true;
			}
		}
		temp = NULL;
	}

	p = NULL;
	if (multiple) {
		char *temp_var = temp_history2[index];
		strcpy(matches, temp_var);
		temp_var = NULL;
	} else {
		strcpy(matches, "");
	}

	if (strcmp(matches, "") == 0) {
		return NULL;
	} else {
		size_t len2 = strlen(matches);
		if (len2 > 0 && matches[len2-1] == '\n') {
			matches[--len2] = '\0';
		}

		start = strcspn(matches, " ");
		memset(letter, 0, 2);
		letter[0] = matches[start + 1];
		char *q = &letter[0];
		q = strstr(matches, q);

		memset(search_result, 0, _POSIX_ARG_MAX);
		strcpy(search_result, q);
		q = NULL;
	}

	return search_result;
}

char *num_search(char *num, int hist_tracker) {
	char command[10000];
	memset(command, 0, 10000);
	int end = strcspn(num, " ");

	for (int i = 1; i < end; i++) {
		append(command, num[i]);
	}

	char temp_history3[HIST_MAX][_POSIX_ARG_MAX];
	memcpy(temp_history3, history, sizeof(history));

	char compare[1000];
	memset(compare, 0, 1000);
	int index = 0;
	int convert = atoi(command);
	bool found = false;
	char matches[_POSIX_ARG_MAX];
	memset(matches, 0, _POSIX_ARG_MAX);

	for (int i = 0; i < HIST_MAX; i++) {
		char *temp = temp_history3[i];
		char sub_str[_POSIX_ARG_MAX];
		memset(sub_str, 0, _POSIX_ARG_MAX);

		while (*temp != ' ') {
			append(sub_str, *temp);
			++temp;
		}

		int temp_num = atoi(sub_str);
		if (temp_num == convert) {
			index = i;
			found = true;
			break;
		}
	}


	char *temp_var = temp_history3[index];
	strcpy(matches, temp_var);

	if (strcmp(matches, "") == 0 || found == false) {
		return NULL;
	} else {
		memset(search_result, 0, _POSIX_ARG_MAX);

		size_t len2 = strlen(matches);
		if (len2 > 0 && matches[len2-1] == '\n') {
			matches[--len2] = '\0';
		}

		int start = strcspn(matches, " ");
		char letter[2];
		memset(letter, 0, 2);
		letter[0] = matches[start + 1];
		char *p = &letter[0];
		p = strstr(matches, p);
		strcpy(search_result, p);
		p = NULL;

		return search_result;
	}
}