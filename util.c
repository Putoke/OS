#include "util.h"
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void split_string(char ** result, char * input, const char delimiter) {

	char * pch;
	int i = 0;
	pch = strtok (input, &delimiter);

	while (pch != NULL)	{
		result[i] = pch;
		pch = strtok (NULL, &delimiter);
		++i;
	}
	result[i] = NULL;

}

char * str_replace(char * str, char * orig, char * rep) {
	static char buffer[1024];
	char *p;

	if(!(p = strstr(str, orig)))
		return str;
	strncpy(buffer, str, p-str);
	buffer[p-str] = '\0';

	sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

	return buffer;
}

void close_a_pipe(int pipe[2]) {
	int ret_value = close(pipe[PIPE_READ_SIDE]); /* Close the read side of the pipe */
	if(ret_value == -1) {
		perror("Cannot close read end"); exit(1);
	}
	ret_value = close(pipe[PIPE_WRITE_SIDE]); /* Close the write side of the pipe */
	if(ret_value == -1) {
		perror("Cannot close write end"); exit(1);
	}
}

void close_pipes(int args, ...) {
	va_list valist;
	int i=0;
	va_start(valist, args);
	while(i < args) { /* iterate through the pipes and close them */
		int *pipe = va_arg(valist, int*);
		close_a_pipe(pipe); 
		++i;
	}
	va_end(valist);
}
