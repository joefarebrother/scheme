/* 
 * A quick and dirty scheme interpreter for bootstrapping the 
 * compiler for the first time. Has pairs, lambdas, strings, 
 * integers, characters, symbols, and IO, no vectors or macros
 * as they are not needed by the compiler. Includes a very simple 
 * reference-counting GC that will leak memory on circular 
 * structures, which is ok because only one (the global 
 * enviroment) is used. Based on SICP.
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

static object *false_obj = &(object){scm_bool, 0, {.i=0}};
static object *true_obj = &(object){scm_bool, 0, {.i=1}};
static object *empty_list = &(object){scm_empty_list, 0, {.i=2}};

int check_type(enum obj_type type, object *obj, int err_on_false)
{
	int result = (type == obj->type);
	if (!result && err_on_false){
		fprintf(stderr, "Type error\n");
		exit(1);
	}
	return result;
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


/*
 * Read
 */



/*
 * Eval
 */



/*
 * Print
 */




/*
 * Loop
 */
int main(void)
{
	/* stub */
	printf("Not implemented yet\n");
	return 0;
}