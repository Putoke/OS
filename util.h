#ifndef UTIL_H
#define UTIL_H

#include <string.h>

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


#endif
