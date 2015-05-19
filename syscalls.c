#define _POSIX_SOURCE

#include "syscalls.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "util.h"

void terminate() {
	int return_value;
	return_value = kill(0, SIGTERM); /* terminate all child processes */
	if(return_value == -1) {
		perror("Termination of child processes failed.");
	}
	exit(0); /* exit the shell */
}

void cd(char * input) {
	int return_value;
	if(input != NULL)
		input = str_replace(input, "~", getenv("HOME"));
	else
		input = getenv("HOME");
	return_value = chdir(input); /* change directory */
	if(return_value == -1) {
		fprintf(stderr, "%s is not a directory\n", input);
	}
}

void check_env(char ** args, char * pager) {
	int pipe_filedesc[2];
	int pipe2_filedesc[2];
	int pipe3_filedesc[2];
	pid_t child_pid;
	int ret_value;
	int status;
	int i = 0;
	
	/* init pipes */
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

	child_pid = fork(); /* create the first child process (printenv) */
	if(child_pid == 0) {
		/*Child process*/
		ret_value = dup2(pipe_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO); /* duplicate first pipe's file descriptor (write side) and replace stdout */
		if (ret_value == -1) {
			perror("Cannot dup"); exit(1);
		}
		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc); /* close all pipes */

		(void) execlp("printenv", "printenv", (char *) 0); /* execute printenv */
		perror("Cannot exec printenv"); exit(1); /* if execlp returns an error has occured */
	}

	if(child_pid == -1) { /* if child_pid is -1 then an error has occured */
		char * errormessage = "UNKNOWN";
		if( EAGAIN == errno ) errormessage = "cannot allocate page table";
		if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
		fprintf( stderr, "fork() failed because: %s\n", errormessage );
	}

	child_pid = fork(); /* create the second child process (grep) */
	if(child_pid == 0) {
		/*Child process*/
		ret_value = dup2(pipe_filedesc[PIPE_READ_SIDE], STDIN_FILENO); /* duplicate first pipe's file descriptor (read side) and replace stdin */
		if (ret_value == -1) {
			perror("Cannot dup"); exit(1);
		}
		ret_value = dup2(pipe2_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO); /* duplicate second pipe's file descriptor (write side) and replace stdout */
		if (ret_value == -1) {
			perror("Cannot dup"); exit(1);
		}

		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc); /* close all pipes */

		args[0] = "grep";
		if(args[1] != '\0') { /* if there are arguments passed then execute grep with arguments */
			(void) execvp(args[0], args);
			perror("Cannot exec grep"); exit(1); /* if execlp returns an error has occured */
		}
		execlp("grep", "grep", "", (char *) 0); /* execute grep withour arguments */
		perror("Cannot exec grep"); exit(1); /* if execlp returns an error has occured */
	}

	if(child_pid == -1) { /* if child_pid is -1 then an error has occured */
		char * errormessage = "UNKNOWN";
		if( EAGAIN == errno ) errormessage = "cannot allocate page table";
		if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
		fprintf( stderr, "fork() failed because: %s\n", errormessage );
	}

	child_pid = fork(); /* create the third child process (sort) r*/
	if(child_pid == 0) {
		/*Child process*/
		ret_value = dup2(pipe2_filedesc[PIPE_READ_SIDE], STDIN_FILENO); /* duplicate second pipe's file descriptor (read side) and replace stdin */
		if (ret_value == -1) {
			perror("Cannot dup"); exit(1);
		}
		ret_value = dup2(pipe3_filedesc[PIPE_WRITE_SIDE], STDOUT_FILENO); /* duplicate third pipe's file descriptor (write side) and replace stdout */
		if (ret_value == -1) {
			perror("Cannot dup"); exit(1);
		}

		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc); /* close all pipes */

		(void) execlp("sort", "sort", (char *) 0); /* execute sort */
		perror("Cannot exec sort"); exit(1); /* if execlp returns an error has occured */
	}

	if(child_pid == -1) { /* if child_pid is -1 then an error has occured */
		char * errormessage = "UNKNOWN";
		if( EAGAIN == errno ) errormessage = "cannot allocate page table";
		if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
		fprintf( stderr, "fork() failed because: %s\n", errormessage );
	}

	child_pid = fork(); /* create the fourth child proces (pager) */
	if(child_pid == 0) {
		/*Child process*/
		ret_value = dup2(pipe3_filedesc[PIPE_READ_SIDE], STDIN_FILENO); /* duplicate third pipe's file descriptor (read side) and replace stdin */
		if (ret_value == -1) {
			perror("Cannot dup"); exit(1);
		}

		close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc); /* close all pipes */

		if(pager != NULL) { /* if the pager variable is defined then execute it  */
			(void) execlp(pager, pager, (char *) 0);
			perror("Cannot exec pager"); exit(1); /* if execlp returns an error has occured */
		}

		(void) execlp("less", "less", (char *) 0); /* execute less if no pager is defined */
		perror("Cannot exec less"); /* if execlp returns an error has occured */

		(void) execlp("more", "more", (char *) 0); /* execute more if less couldn't be executed */
		perror("Cannot exec more"); exit(1); /* if execlp returns an error has occured */
	}

	if(child_pid == -1) { /* if child_pid is -1 then an error has occured */
		char * errormessage = "UNKNOWN";
		if( EAGAIN == errno ) errormessage = "cannot allocate page table";
		if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
		fprintf( stderr, "fork() failed because: %s\n", errormessage );
	}

	close_pipes(3, pipe_filedesc, pipe2_filedesc, pipe3_filedesc); /* close all pipes */

	/* Wait for all children processes to finish */
	while(i < 4) {
		child_pid = wait( &status ); /* wait for child */ 
		error_helper(child_pid, status); /* print errors */
		++i;
	}
}

void error_helper(pid_t child_pid, int status) {
	if( -1 == child_pid ) {
		perror( "wait() failed unexpectedly" ); exit( 1 );
	}

	if( WIFEXITED( status )) { /* child process is finished*/
		int child_status = WEXITSTATUS( status );
		if( 0 != child_status ) { /* child had problems */
			fprintf( stderr, "Child (pid %ld) failed with exit code %d\n", (long int) child_pid, child_status );
		}
	} else {
		if( WIFSIGNALED( status ) ) { /* child process was interrupted by signal */
	    	int child_signal = WTERMSIG( status );
	    	fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n" ,(long int) child_pid, child_signal );
		}
	}

 }
