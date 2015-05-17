#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <unistd.h>
#include <sys/types.h>

/*
	Change the current working directory to the given string.
*/
void cd(const char * input);

/*
	Print environment variables sorted and optionally with grep arguments.
*/
void check_env(char ** args, char * pager);

/*
	Terminate the program by killing its children and then itself.
*/
void terminate(pid_t childpid);

/*
	Handles errors regarding child processes.
*/
void error_helper(pid_t child_pid, int status);

#endif
