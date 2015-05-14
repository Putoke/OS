#ifndef UTIL_H
#define UTIL_H

#define PIPE_READ_SIDE ( 0 )
#define PIPE_WRITE_SIDE ( 1 )

void split_string(char ** arr, char * input);
char * str_replace(char * str, char * orig, char * rep);
void close_a_pipe(int pipe[2]);
void close_pipes(int args, ...);


#endif
