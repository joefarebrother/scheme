/* 
 * A quick and dirty scheme interpreter for bootstrapping the 
 * compiler for the first time. Has pairs, lambdas, strings, 
 * integers, characters, symbols, and IO, all that is 
 * neccasary for the compiler. Includes a very simple 
 * reference-counting GC that will leak memory on circular 
 * structures, which is ok because only one (the global 
 * enviroment) is used. Based on SICP.
 */

#include <stdio.h>

/*
 * DATA
 */

enum obj_type {
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
};

struct object {
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
};

static struct object *false_obj = &(struct object){scm_bool, 0, {.i=0}};
static struct object *true_obj = &(struct object){scm_bool, 0, {.i=1}};
static struct object *empty_list = &(struct object){scm_empty_list, 0, {.i=2}};

static int err_flag = 0;
static char *err_msg = "";

int check_type(enum obj_type type, struct object *obj, int err_on_false){
	int result = (type == obj->type);
	if (!result && err_on_false){
		err_flag = 1;
		err_msg = "Type error";
	}
	return result;
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
int main(void){
	/* stub */
	printf("Not implemented yet\n");
	return err_flag;
}