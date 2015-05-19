#define _POSIX_SOURCE
#define _XOPEN_SOURCE 500

#define TRUE 1
#define FALSE 0

#if defined(SIGDET) && SIGDET+0
	int signalTerm = 1;
#else
	int signalTerm = 0;
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "colors.h"
#include "util.h"
#include "syscalls.h"


void input_handle(char input[]);

void register_sighandler(int signal_code, void (*handler)(int sig));
void cleanup_handler(int signal_code);
void sighold_error(int signal_code);
void sigrelse_error(int signal_code);

void poll();

pid_t childpid;
char *pager;



int main(int args, char ** argv) {
	char cwd[1024]; /* current working directory buffer */
	char input[1024]; /* user input buffer */
	char *uid = getenv("USER"); /* Username */
	char *home = getenv("HOME"); /* Homepath */
	char hostname[1024]; /* hostname */ 

	pager = getenv("PAGER"); /* PAGER env variable */
	/*hostname[1023] = '\0'; */
	gethostname(hostname, 1023); /* get hostname */

	/* Register  signal handlers handlers */
	register_sighandler(SIGINT, cleanup_handler); 
	register_sighandler(SIGCHLD, cleanup_handler);

	sighold_error(SIGTERM); /* Ignore SIGTERM signals */

	while(strcmp(input, "exit")) { /* run until the user wants to exit */
		if(getcwd(cwd, sizeof(cwd)) != NULL) { 			/*Get current working directory*/
			printf(BOLDGREEN	"%s@%s" BOLDBLUE " %s $ "	RESET, uid, hostname, str_replace(cwd, home, "~"));	/*Print the prompt*/
		}
		if(fgets(input, 1023, stdin)){				/*Scan user input*/
			input[strcspn(input, "\r\n")] = 0;		/*Remove trailing newline*/

			if (input[0] != '\0' && input[0] != ' ') /*If input is not empty then handle input*/
				input_handle(input);
			if(!signalTerm) { /* if signalTerm is 0 then start polling for terminated child processes */
				poll();
			}
		}
	}
	
	terminate(childpid); /* When the user has chosen to exit terminate all child processes then terminate the shell */
	return 0;
}

void input_handle(char input[]) {
	char * charv[40]; /* argument buffer */
	int background = FALSE; /* determines background or foreground process */
	split_string(charv, input, ' '); /* split the input string on space to save arguments in buffer */

	if(charv[0][strlen(charv[0])-1] == '&' || (charv[1] != '\0' && strcmp(charv[1], "&") == 0) ) { /*Check if the user wants to create a background process*/
		background = TRUE; 
		if(charv[0][strlen(charv[0])-1] == '&' ) /* if the input ended with a & then remove it from the string */
			charv[0][strlen(charv[0])-1] = '\0'; 
		if((charv[1] != '\0' && strcmp(charv[1], "&") == 0)) /* if the & character was an own anrgument then remove that argument */
			charv[1] = '\0';
		sighold_error(SIGINT); /* ignore SIGINT for background processes */
	} else {
		sighold_error(SIGCHLD); /* ignore IGCHLD for foreground processes */
		sigrelse_error(SIGINT); /* don't ignore SIGINT for foreground processes */
	}

	childpid = fork(); /* create child process */ 

	if(childpid == 0) { /*Child process*/
		sigrelse_error(SIGTERM); /* don't ignore SIGTERM for child processes */
		if(strcmp(charv[0], "checkEnv") == 0) /* if the input is checkEnv then run checkEnv function */
			check_env(charv, pager);
		else if(strcmp(charv[0], "cd") != 0){ /* don't try to execute cd */
			execvp(charv[0], charv); /* execute the input */
			perror("Failed to execute");
			exit(1);
		}
		exit(0); /* terminate the child */

	} else { 			/*Parent process*/
		static struct timeval tm1, tm2; /* structs used to measure process runtime */
		unsigned long t; /* total process runtime */
		int ret_value;

		if(childpid == -1) { /* if childpid is -1 then an error has occured */
			char * errormessage = "UNKNOWN";
			if( EAGAIN == errno ) errormessage = "cannot allocate page table";
			if( ENOMEM == errno ) errormessage = "cannot allocate kernel data";
			fprintf( stderr, "fork() failed because: %s\n", errormessage );
		}
		if (background) {
			if(signalTerm) {
				sigrelse_error(SIGCHLD); /* if signalTerm is defined and the child is a background process don't ignore SIGCHLD */
			}
			return;
		}

		sigrelse_error(SIGINT); /* if the child is a foreground process ignore SIGCHLD */
		
		if(strcmp(charv[0], "cd") == 0) /* if the input is cd then run the cd function */
			cd(charv[1]);

		gettimeofday(&tm1, NULL); /* get the time in millis before waiting for the child */

		ret_value = waitpid(childpid, 0, 0); /* wait for the child to finish */
		if (ret_value == -1 ) {
			perror("Wait failed unexpectedly");
		}


		gettimeofday(&tm2, NULL); /* get the time after the child has terminated */
		t = 1000 * (tm2.tv_sec - tm1.tv_sec) + (tm2.tv_usec - tm1.tv_usec) / 1000; /* calculate total child process runtime */
		printf("Foreground process terminated in %lu milliseconds.\n", t); /* print child process runtimme */
	}
}


/*
	Poll for terminated process
*/
void poll() {
	pid_t child;
	int status;
	while((child = waitpid(-1, &status, WNOHANG)) > 0) { /* Poll for any terminated process */
		printf("Background process %d terminated with status %d\n", child, status);
	}
}

/*
	Signal handler for SIGINT and SIGCHLD
*/
void cleanup_handler(int signal_code) {
	if(childpid > 0 && signal_code == SIGINT) { /* if the signal is of type SIGINT then terminate the child and wait for it to finish */
		int ret_value = kill(childpid, SIGTERM);
		if (ret_value == -1) {
			perror("Failed to kill child process");
		}
		ret_value = waitpid(childpid, 0, 0);
		if (ret_value == -1 ) {
			perror("Wait failed unexpectedly");
		}
	}

	if(childpid > 0 && signal_code == SIGCHLD) { /* if the signal is of type SIGCHLD then start polling for terminated child processes */
		poll();
	}
}


/*
	Register a signalhandler for a given signal.
*/
void register_sighandler(int signal_code, void (*handler)(int sig)) {

	int return_value;

	struct sigaction signal_parameters; /* struct for sigaction parameters */
	signal_parameters.sa_handler = handler; /* set the desired handler */
	sigemptyset( &signal_parameters.sa_mask ); /* we don't need a mask so set it to an empty set */
	signal_parameters.sa_flags = 0; /* we don't need flags so set it to 0*/
	return_value = sigaction( signal_code, &signal_parameters, (void *) 0); /* register the handler */
	
	if ( return_value == -1) { /* sigaction failed */
		 perror( "sigaction() failed" ); exit( 1 );
	}
}

void sighold_error(int signal_code) {
	int return_value = sighold(signal_code);
	if (return_value == -1) {
		if(signal_code == SIGTERM)
			fprintf(stderr, "Could not hold SIGTERM\n");
		else if (signal_code == SIGCHLD)
			fprintf(stderr, "Could not hold SIGCHLD\n");
		else if(signal_code == SIGINT)
			fprintf(stderr, "Could not hold SIGINT\n");
		exit(1);
	}
}

void sigrelse_error(int signal_code) {
	int return_value = sigrelse(signal_code);
	if (return_value == -1) {
		if(signal_code == SIGTERM)
			fprintf(stderr, "Could not hold SIGTERM\n");
		else if (signal_code == SIGCHLD)
			fprintf(stderr, "Could not hold SIGCHLD\n");
		else if(signal_code == SIGINT)
			fprintf(stderr, "Could not hold SIGINT\n");
		exit(1);
	}
}
