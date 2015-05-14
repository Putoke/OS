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
	pid_t child_pid;
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

	child_pid = fork();
	if(child_pid == 0) {
		/*Child process*/
		dup2(pipe_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO);

		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		(void) execlp("printenv", "printenv", (char *) 0);
		perror("Cannot exec printenv"); exit(1);
	}

	child_pid = fork();
	if(child_pid == 0) {
		/*Child process*/
		dup2(pipe_filedesc[PIPE_READ_SIDE], STDIN_FILENO);
		dup2(pipe2_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO);

		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		args[0] = "grep";
		if(args[1] != '\0') {
			(void) execvp(args[0], args);
			perror("Cannot exec grep"); exit(1);
		}
		execlp("grep", "grep", "", (char *) 0);
	}

	child_pid = fork();
	if(child_pid == 0) {
		/*Child process*/
		dup2(pipe2_filedesc[PIPE_READ_SIDE], STDIN_FILENO);
		dup2(pipe3_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO);

		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		(void) execlp("sort", "sort", (char *) 0);
		perror("Cannot exec sort"); exit(1);
	}

	child_pid = fork();
	if(child_pid == 0) {
		/*Child process*/
		dup2(pipe3_filedesc[PIPE_READ_SIDE], STDIN_FILENO);

		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

		(void) execlp("less", "less", (char *) 0);
		perror("Cannot exec less"); exit(1);
	}

	close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc);

	child_pid = wait(&status);
	child_pid = wait(&status);
	child_pid = wait(&status);
	child_pid = wait(&status);
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


