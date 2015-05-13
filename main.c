#define _POSIX_SOURCE
#define _BSD_SOURCE
#define PIPE_READ_SIDE ( 0 )
#define PIPE_WRITE_SIDE ( 1 )

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
void check_env(char ** args);

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
	int return_value;
	return_value = chdir(input);
	if(return_value == -1) {
		printf("Not a directory\n");
	}
}

void check_env(char ** args) {
	int pipe_filedesc[2];
	int pipe2_filedesc[2];
	int pipe3_filedesc[2];
	pid_t childpid;
	int ret_value;
	int status;
	
	ret_value = pipe(pipe_filedesc);
	if (ret_value == -1) {
		perror("cannot create pipe"); exit(1);
	}
	ret_value = pipe(pipe2_filedesc);
	if (ret_value == -1) {
		perror("cannot create pipe 2"); exit(1);
	}
	ret_value = pipe(pipe3_filedesc);
	if (ret_value == -1) {
		perror("cannot create pipe 3"); exit(1);
	}

	childpid = fork();
	if(childpid == 0) {
		/*Child process*/
		dup2(pipe_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO);

		close_pipe(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		(void) execlp("printenv", "printenv", (char *) 0);
		perror("Cannot exec printenv"); exit(1);
	}

	childpid = fork();
	if(childpid == 0) {
		/*Child process*/
		dup2(pipe_filedesc[PIPE_READ_SIDE], STDIN_FILENO);
		dup2(pipe2_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO);

		close_pipe(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		args[0] = "grep";
		if(args[1] != '\0') {
			(void) execvp(args[0], args);
			perror("Cannot exec grep"); exit(1);
		}
		execlp("grep", "grep", "", (char *) 0);
	}

	childpid = fork();
	if(childpid == 0) {
		/*Child process*/
		dup2(pipe2_filedesc[PIPE_READ_SIDE], STDIN_FILENO);
		dup2(pipe3_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO);

		close_pipe(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		(void) execlp("sort", "sort", (char *) 0);
		perror("Cannot exec sort"); exit(1);
	}

	childpid = fork();
	if(childpid == 0) {
		/*Child process*/
		dup2(pipe3_filedesc[PIPE_READ_SIDE], STDIN_FILENO);

		close_pipe(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		(void) execlp("less", "less", (char *) 0);
		perror("Cannot exec less"); exit(1);
	}

	close_pipe(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

	childpid = wait(&status);
	childpid = wait(&status);
	childpid = wait(&status);
	childpid = wait(&status);
}



