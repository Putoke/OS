#define _POSIX_SOURCE
#define _BSD_SOURCE

#define TRUE 1
#define FALSE 0

#define SIGDET 1

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "colors.h"
#include "util.h"
#include "syscalls.h"



void kill_child(pid_t child_id);
void input_handle(char input[]);

void register_sighandler(int signal_code, void (*handler)(int sig));
void signal_handler(int signal_code);
void cleanup_handler(int signal_code);

void poll();

pid_t childpid;

int main(int args, char ** argv) {
	char cwd[1024], input[81];
	char *uid = getenv("USER");
	char *home = getenv("HOME");
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	register_sighandler(SIGINT, cleanup_handler);
	register_sighandler(SIGCHLD, cleanup_handler);
	sighold(SIGTERM);

	while(strcmp(input, "exit")) {
		if(getcwd(cwd, sizeof(cwd)) != NULL) { 			/*Get current working directory*/
			printf(BOLDGREEN	"%s@%s" BOLDBLUE " %s $ "	RESET, uid, hostname, str_replace(cwd, home, "~"));	/*Print the prompt*/
		}
		if(fgets(input, 80, stdin)){				/*Scan user input*/
			input[strcspn(input, "\r\n")] = 0;		/*Remove trailing newline*/

			if (input[0] != '\0')
				input_handle(input);
			if(!SIGDET) {
				poll();
			}
		}
	}
	
	terminate(childpid);
	return 0;
}

void input_handle(char input[]) {
	char * charv[10];
	int background = FALSE;
	split_string(charv, input);

	if(charv[0][strlen(charv[0])-1] == '&' || (charv[1] != '\0' && strcmp(charv[1], "&") == 0) ) { /*Check if the user wants to create a background process*/
		background = TRUE;
		if(charv[0][strlen(charv[0])-1] == '&' )
			charv[0][strlen(charv[0])-1] = '\0';
		if((charv[1] != '\0' && strcmp(charv[1], "&") == 0))
			charv[1] = 0; 
	}
	
	if(!background) {
		sighold(SIGCHLD);
		sigrelse(SIGINT);
	} else {
		sighold(SIGINT);
	}

	childpid = fork();
		
	if(childpid == 0) { /*Child process*/
		sigrelse(SIGTERM);
		if(strcmp(charv[0], "checkEnv") == 0)
			check_env(charv);
		else {
			execvp(charv[0], charv);
		}
		exit(0);

	} else { 			/*Parent process*/
		static struct timeval tm1, tm2;
		unsigned long t;

		if(childpid == -1) {
			char * errormessage = "UNKNOWN";
			if( EAGAIN == errno ) errormessage = "cannot allocate page table";
			if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
			fprintf( stderr, "fork() failed because: %s\n", errormessage );
		}
		if (background) {
			if(SIGDET) {
				sigrelse(SIGCHLD);
			}
			return;
		}

		sigrelse(SIGINT);
		
		if(strcmp(charv[0], "cd") == 0)
			cd(charv[1]);

		gettimeofday(&tm1, NULL);

		waitpid(childpid, 0, 0);

		gettimeofday(&tm2, NULL);
		t = 1000 * (tm2.tv_sec - tm1.tv_sec) + (tm2.tv_usec - tm1.tv_usec) / 1000;
		printf("Foreground process terminated in %lu milliseconds.\n", t);
	}
}

void poll() {
	pid_t child;
	int status;
	while((child = waitpid(-1, &status, WNOHANG)) > 0) {
		printf("Background process %d terminated with status %d\n", child, status);
	}
}

void kill_child(pid_t child_id) {
	kill(child_id, SIGTERM);
}

void cleanup_handler(int signal_code) {

	if(childpid > 0 && signal_code == SIGINT) {
		kill_child(childpid);
		waitpid(childpid, 0, 0);
	}

	if(childpid > 0 && signal_code == SIGCHLD) {
		poll();
	}
		
}

void register_sighandler(int signal_code, void (*handler)(int sig)) {
	struct sigaction signal_parameters;
	signal_parameters.sa_handler = handler;
	sigemptyset( &signal_parameters.sa_mask );
	signal_parameters.sa_flags = 0;
	sigaction( signal_code, &signal_parameters, (void *) 0);
}


