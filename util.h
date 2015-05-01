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


#endif
