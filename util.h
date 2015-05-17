#ifndef UTIL_H
#define UTIL_H

#define PIPE_READ_SIDE ( 0 ) /* Define the pipe read side */
#define PIPE_WRITE_SIDE ( 1 ) /* Define the pipe write side */

/*
	Split a string by delemiter, store the results in an array of strings.
*/
void split_string(char ** result, char * input, const char delimiter);

/*
	Replace all occurrences of a character with another i a string.
*/
char * str_replace(char * str, char * orig, char * rep);

/*
	Close a pipe by closing PIPE_READ_SIDE and PIPE_WRITE_SIDE.
*/
void close_a_pipe(int pipe[2]);

/*
	Close a variadic amount of pipes.
*/
void close_pipes(int args, ...);


#endif
