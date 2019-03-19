#define _PROGRAM_NAME "whoami"

#define BUFF 256

char HOME[BUFF];
char USERNAME[BUFF];
char HOSTNAME[BUFF];

void get_user();
void get_hostname();
void get_home_dir();