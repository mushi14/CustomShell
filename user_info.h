#define _PROGRAM_NAME "whoami"

#define HOST_BUFF 256

char HOME[100];
char USERNAME[100];
char HOSTNAME[100];

void get_user();
void get_hostname();
void get_home_dir();