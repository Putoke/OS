#define _POSIX_SOURCE

#include "syscalls.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "util.h"

int terminate(pid_t childpid) {

	kill(0, SIGTERM);
	exit(0);
	return 0;
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
