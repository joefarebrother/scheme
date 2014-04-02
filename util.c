#include "util.h"
#include <stdlib.h>
#include <string.h>

char peek(FILE *in){
	char c = getc(in);
	ungetc(c, in);
	return c;
}


void eat_ws(FILE *in)
{
	int c;
	while((c = getc(in)) != EOF){
		if (isspace(c)) continue;
		else if (c == ';'){  /* comment - ignore */
			while((c = getc(in)) != '\n' && c != EOF);
			continue;
		}
		ungetc(c, in);
		break;
	}
}

void eat_expected_str(FILE *in, char *str)
{
	char c;

	while(*str != '\0'){
		c = getc(in);
		if(c != *str++){
			fprintf(stderr, "Unexpected character: expecting %s, got %c\n", str, c);
			exit(1);
		}
	}
}

#define INT2STR_BUFLEN 12 /* a 32 bit integer is never longer than 12 digits in decimal */
char *int_to_string(int num)
{
	char buf[INT2STR_BUFLEN];
	snprintf(buf, INT2STR_BUFLEN, "%d", num);
	return strdup(buf);
}

char *str_append(char* first, char *second){
	char *buf = malloc(strlen(first) + strlen(second) + 1); /* +1 for null character */
	char *ret;
	if(buf == NULL)
		return NULL;

	strcpy(buf, first);
	strcat(buf, second);

	ret = strdup(buf);
	free(buf);
	return ret;
}