/* 
 * A quick and dirty scheme interpreter for bootstrapping the 
 * compiler for the first time. Has pairs, lambdas, strings, 
 * integers, characters, symbols, and IO, but no vectors or macros 
 * or continuations as they are not needed by the compiler. 
 * Includes a very simple reference-counting GC that will leak 
 * memory on circular structures, which is ok because only one 
 * (the global enviroment) is used. Based on SICP and 
 * http://michaux.ca/articles/scheme-from-scratch-introduction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * DATA
 */

typedef enum obj_type {
	scm_bool,
	scm_empty_list,
	scm_char,
	scm_int,
	scm_pair,
	scm_symbol,
	scm_prim_fun,
	scm_lambda,
	scm_str,
	scm_file
} obj_type;

typedef struct object {
	enum obj_type type;
	int refs;
	union {
		char c;
		int i;
		struct {
			struct object *car;
			struct object *cdr;
		} pair;
		char *str;
		struct object (**prim)(struct object *);
		struct {
			struct object *env;
			struct object *args;
			struct object *code;
		} lambda;
		FILE *file;
	} data;
} object;

object *true;
object *false;
object *empty_list;
object *std_input;
object *std_output;


int check_type(enum obj_type type, object *obj, int err_on_false)
{
	int result = (type == obj->type);
	if (!result && err_on_false){
		fprintf(stderr, "Type error\n");
		exit(1);
	}
	return result;
}

int is_true(object *obj)
{
	return obj != false;
}

object *alloc_obj(void)
{
	object *obj;
	obj = malloc(sizeof(object));
	if (obj == NULL)
	{
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	}
	obj->refs = 0;
	return obj;
}

void init_constants(void)
{
	true = alloc_obj();
	true->type = scm_bool;
	true->data.i = 1;

	false = alloc_obj();
	false->type = scm_bool;
	false->data.i = 0;

	empty_list = alloc_obj();
	empty_list->type = scm_empty_list;

	std_input = alloc_obj();
	std_input->type = scm_file;
	std_input->data.file = stdin;

	std_output = alloc_obj();
	std_output->type = scm_file;
	std_output->data.file = stdout;
}

object *make_int(int value)
{
	object *obj = alloc_obj();
	obj->type = scm_int;
	obj->data.i = value;
	return obj;
}

int obj2int(object *obj)
{
	check_type(scm_int, obj, 1);
	return obj->data.i;
}

object *make_bool(int value)
{
	return value ? true : false;
}

int obj2bool(object *obj)
{
	check_type(scm_bool, obj, 1);
	return obj->data.i;
}

object *make_char(char c)
{
	object *obj = alloc_obj();
	obj->type = scm_char;
	obj->data.c = c;
	return obj;
}

char obj2char(object *obj)
{
	check_type(scm_char, obj, 1);
	return obj->data.c;
}

object *make_str(char *str)
{
	object *obj = alloc_obj();
	obj->type = scm_str;
	obj->data.str = malloc(strlen(str) + 1);
	if (obj->data.str = NULL){
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	}
	strcpy(obj->data.str, str);
	return obj;
}

char *obj2str(object *obj)
{
	check_type(scm_str, obj, 1);
	return obj->data.str;
}

/*
 * Read
 */

int is_delimiter(char c)
{
	return isspace(c) || c == EOF ||
		   c == '('   || c == ')' ||
		   c == '"'   || c == ';';
}

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
			while((c = getc(in)) != '\n');
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

void expect_delim(FILE *in)
{
	if(!is_delimiter(peek(in))){
		fprintf(stderr, "Unexpected character: expecting a delimiter, got %c.\n", peek(in));
		exit(1);
	}
}

object *read_char(FILE *in) 
{
    int c;

    c = getc(in);
    switch (c) {
    case EOF:
        fprintf(stderr, "Incomplete character literal.\n");
        exit(1);
    case 's':
        if (peek(in) == 'p') {
            eat_expected_str(in, "pace");
            c = ' ';
        }
        break;
    case 'n':
        if (peek(in) == 'e') {
            eat_expected_str(in, "ewline");
            c = '\n';
        }
        break;
    case 't':
    	if (peek(in) == 'a'){
    		eat_expected_str(in, "ab");
    		c = '\t';
    	}
    	break;
    }
    expect_delim(in);
    return make_char(c);
}

object *read(FILE *in)
{
	int c; 

	eat_ws(in);

	c = getc(in);
	if (isdigit(c) || (c == '-' && isdigit(peek(in))))
	{
		/* read an integer */
		int sign = 1, num = 0;
		if (c == '-')
			sign = -1;
		else 
			ungetc(c, in);
		

		while (isdigit(c = getc(in)))
			num = (num * 10) + (int)c - '0';

		num *= sign;

		if (!is_delimiter(c)) {
			fprintf(stderr, "Number did not end with a delimiter\n");
			exit(1);
		}

		return make_int(num);
	}
	else if (c == '#'){
		/* read boolean or character */
		switch(c = getc(in)){
		case 't':
			return true;
		case 'f':
			return false;
		case '\\':
			return read_char(in);
		default:
			fprintf(stderr, "Bad input. Expecting t, f, or \\, got %c.\n", c);
			exit(1);
		}
	}
	else{
		fprintf(stderr, "Bad input. Unexpected %c.\n", c);
		exit(1);
	}
}


/*
 * Eval
 */

object *eval(object *code)
{
	/* Until we have lists and symbols just echo */
	return code;
}

/*
 * Print
 */

void print(FILE *out, object *obj)
{
	switch(obj->type) {
	case scm_int:
		fprintf(out, "%d", obj2int(obj));
		break;
	case scm_bool:
		fprintf(out, obj2bool(obj) ? "#t" : "#f");
		break;
	case scm_char: { /* curly brace required for scope of variable c*/
		char c = obj2char(obj);
		fprintf(out, "#\\");
		switch(c) {
		case ' ':
			fprintf(out, "space");
			break;
		case '\n':
			fprintf(out, "newline");
			break;
		case '\t':
			fprintf(out, "tab");
			break;
		default:
			fputc(c, out);
		}
		break;
	}
	default:
		fprintf(stderr, "Unkown data type in write\n");
		exit(1);
	}
}


/*
 * REPL
 */
int main(void)
{
	printf("Welcome to bootstrap scheme. \n%s",
		  "Press ctrl-c to exit. \n");

	init_constants();

	while(1){
		printf("> ");
		print(stdout, eval(read(stdin)));
		printf("\n");
	}
}