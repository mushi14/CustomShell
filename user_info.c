#include "user_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h> 

/**
 * Gets the username
 */
void get_user() {
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	if (pw) {
		strcpy(USERNAME, pw->pw_name);
	}
}

/**
 * Gets the hostname of the system
 */
void get_hostname() {
	char buff[BUFF];
	int hostname;

	hostname = gethostname(buff, sizeof(buff));
	if (hostname != -1) {
		strcpy(HOSTNAME, buff);
	} else {
		perror("Hostname is invalid.");
	}
}

/**
 * Gets the home directory of the account
 */ 
void get_home_dir() {
	struct passwd *pw = getpwuid(getuid());
	strcpy(HOME, pw->pw_dir);
}