#define _POSIX_SOURCE
#define _BSD_SOURCE


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "colors.h"
#include "util.h"
#include "syscalls.h"



void kill_child(pid_t child_id);
void input_handle(char input[]);

void register_sighandler(int signal_code, void (*handler)(int sig));
void signal_handler(int signal_code);
void cleanup_handler(int signal_code);

pid_t childpid;

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

			if (input[0] != '\0')
				input_handle(input);
		}
	}
	
	terminate();
	return 0;
}

void input_handle(char input[]) {
	char * charv[10];
	childpid = fork();
	split_string(charv, input);
		
	if(childpid == 0) { /*Child process*/
		register_sighandler(SIGINT, signal_handler);
		if(strcmp(charv[0], "checkEnv") == 0)
			check_env(charv);
		else
			execvp(charv[0], charv);
		exit(0);
	} else { 			/*Parent process*/
		if(childpid == -1) {
			char * errormessage = "UNKNOWN";
			if( EAGAIN == errno ) errormessage = "cannot allocate page table";
			if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
			fprintf( stderr, "fork() failed because: %s\n", errormessage );
		}
		register_sighandler(SIGINT, cleanup_handler);
		if(strcmp(charv[0], "cd") == 0)
			cd(charv[1]);
		wait(NULL);
	}
}



void kill_child(pid_t child_id) {
	kill(child_id, SIGKILL);
}





void signal_handler(int signal_code){

}

void cleanup_handler(int signal_code) {
	if(childpid > 0 && signal_code == SIGINT) {
		kill(childpid, SIGKILL);
	}
}

void register_sighandler(int signal_code, void (*handler)(int sig)) {
	struct sigaction signal_parameters;
	signal_parameters.sa_handler = handler;
	sigemptyset( &signal_parameters.sa_mask );
	signal_parameters.sa_flags = 0;
	sigaction( signal_code, &signal_parameters, (void *) 0);
}


