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
		char ch;
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

static object *the_false_obj = &(object){scm_bool, 0, {.i=0}};
static object *the_true_obj = &(object){scm_bool, 0, {.i=1}};
static object *the_empty_list = &(object){scm_empty_list, 0, {.i=2}};

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
	return obj != the_false_obj;
}

object *alloc_obj(void)
{
	object *obj;
	obj = malloc(sizeof(object));
	if (obj == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	obj->refs = 0;
	return obj;
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
	return value ? the_true_obj : the_false_obj;
}

int obj2bool(object *obj)
{
	check_type(scm_bool, obj, 1);
	return obj->data.i;
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
	char c;
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

object *read(FILE *in)
{
	char c; 

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
		/* read boolean (or, later, a character) */
		switch(c = getc(in)){
		case 't':
			return the_true_obj;
		case 'f':
			return the_false_obj;
		default:
			fprintf(stderr, "Bad input. Expecting t or f, got %c.\n", c);
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
	switch(obj->type){
	case scm_int:
		fprintf(out, "%d", obj2int(obj));
		break;
	case scm_bool:
		fprintf(out, obj2bool ? "#t" : "#f");
		break;
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
		  "Press ctrl-c to exit");

	while(1){
		printf(">");
		print(stdout, eval(read(stdin)));
		printf("\n");
	}
}