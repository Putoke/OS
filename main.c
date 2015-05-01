#define _POSIX_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "colors.h"
#include "util.h"

int terminate();
void kill_child(pid_t child_id);
void input_handle(char input[]);

int main(int args, char ** argv) {
	char cwd[1024], input[81];

	while(strcmp(input, "exit")) {
		if(getcwd(cwd, sizeof(cwd)) != NULL) { 			/*Get current working directory*/
			printf(BOLDGREEN	"%s (%d) $ "	RESET, cwd, getpid());	/*Print the prompt*/
		}
		if(fgets(input, 80, stdin)){				/*Scan user input*/
			input[strcspn(input, "\r\n")] = 0;		/*Remove trailing newline*/
			input_handle(input);
		}
	}
	
	terminate();
	return 0;
}

void input_handle(char input[]) {
	pid_t childpid = fork();
		
	if(childpid == 0) { /*Child process*/
		char * charv[10];
		split_string(charv, input);
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
