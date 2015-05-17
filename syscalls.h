#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <unistd.h>
#include <sys/types.h>

/*
	Change the current working directory to the given string.
*/
void cd(const char * input);

/*
	TODO
*/
void check_env(char ** args);

/*
	Terminate the program by killing its children and then itself.
*/
int terminate(pid_t childpid);

#endif
