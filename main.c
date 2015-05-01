#define _POSIX_SOURCE
#define _BSD_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include "colors.h"
#include "util.h"

int terminate();
void kill_child(pid_t child_id);
void input_handle(char input[]);

int main(int args, char ** argv) {
	char cwd[1024], input[81];
	char *uid = getenv("USER");
	char *home = getenv("HOME");
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	while(strcmp(input, "exit")) {
		if(getcwd(cwd, sizeof(cwd)) != NULL) { 			/*Get current working directory*/
			printf(BOLDGREEN	"%s@%s" BOLDBLUE " %s $ "	RESET, uid, hostname, str_replace(cwd, home, "~"));	/*Print the prompt*/
		}
		if(fgets(input, 80, stdin)){				/*Scan user input*/
			input[strcspn(input, "\r\n")] = 0;		/*Remove trailing newline*/
			input_handle(input);
		}
	}
	
	terminate();
	return 0;
}

char * find_command(char * folder, char * str){
	DIR *d;
	static char return_string[1024] = {'\0'};
	struct dirent *dir;
	d = opendir(folder);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
		  if(strcmp(str, dir->d_name) == 0){
		  	strcat(return_string, folder);
		  	strcat(return_string, str);
		  }
		}

		closedir(d);
	}
	if(return_string[0] == '\0')
		return str;
	return return_string;
}

void input_handle(char input[]) {
	pid_t childpid = fork();
		
	if(childpid == 0) { /*Child process*/
		char * charv[10];
		split_string(charv, input);
		charv[0] = find_command("/usr/bin/", charv[0]);
		charv[0] = find_command("/bin/", charv[0]);
		execv(charv[0], charv);
		exit(0);
	} else { 			/*Parent process*/
		if(childpid == -1) {
			char * errormessage = "UNKNOWN";
			if( EAGAIN == errno ) errormessage = "cannot allocate page table";
			if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
			fprintf( stderr, "fork() failed because: %s\n", errormessage );
		}
		wait(NULL);
	}
}

int terminate() {

	kill(0, SIGTERM);
	return 0;
}

void kill_child(pid_t child_id) {
	kill(child_id, SIGKILL);
}
