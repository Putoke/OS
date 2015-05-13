#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <stdarg.h>

void split_string(char ** arr, char * input) {

	char * pch;
	int i = 0;
	pch = strtok (input," ");

	while (pch != NULL)	{
		arr[i] = pch;
		pch = strtok (NULL, " ");
		++i;
	}
	arr[i] = NULL;

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
	int ret_value = close(pipe[PIPE_READ_SIDE]);
	if(ret_value == -1) {
		perror("Cannot close read end"); exit(1);
	}
	ret_value = close(pipe[PIPE_WRITE_SIDE]);
	if(ret_value == -1) {
		perror("Cannot close write end"); exit(1);
	}
}

void close_pipe(int args, ...) {
	va_list valist;
	int i=0;
	va_start(valist, args);
	while(i < args) {
		int *pipe = va_arg(valist, int*);
		close_a_pipe(pipe);
		++i;
	}
}

#endif
