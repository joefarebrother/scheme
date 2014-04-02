#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>


void eat_ws(FILE *in);
char peek(FILE *in);
void eat_expected_str(FILE *in, char *str);
char *int_to_string(int num);
char *str_append(char* first, char *second);

#endif