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
void cd(const char * input);
void check_env(const char ** args);

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

void input_handle(char input[]) {
	pid_t childpid = fork();
	char * charv[10];
	split_string(charv, input);
		
	if(childpid == 0) { /*Child process*/
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
		if(strcmp(charv[0], "cd") == 0)
			cd(charv[1]);
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

void cd(const char * input) {
	chdir(input);
}

void check_env(const char ** args) {
	int pipe_filedesc[2];
	pid_t childpid = fork();
	
	pipe(pipe_filedesc);
	if(childpid == 0) {
		/*Child process*/
		dup2(pipe_filedesc[1], STDOUT_FILENO);
		close(pipe_filedesc[0]);
		close(pipe_filedesc[1]);
		execlp("printenv", "printenv", (char *) 0);
	}
	childpid = fork();
	if(childpid == 0) {
		/*Child process*/
		dup2(pipe_filedesc[0], STDIN_FILENO);
		close(pipe_filedesc[1]);
		close(pipe_filedesc[0]);
		execlp("sort", "sort", NULL);
	}

}



