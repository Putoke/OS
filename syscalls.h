#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <unistd.h>
#include <sys/types.h>
void cd(const char * input);
void check_env(char ** args);
int terminate(pid_t childpid);

#endif
