#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <stdio.h>
#include "cxrs.h"

typedef struct object object;

object *true;
object *false;
object *eof;
object *empty_list;
object *global_enviroment;

enum obj_type {
	scm_bool,
	scm_empty_list,
	scm_eof,
	scm_char,
	scm_int,
	scm_pair,
	scm_symbol,
	scm_prim_fun,
	scm_lambda,
	scm_str,
	scm_file
};

typedef object *(*prim_proc)(object *args);

object *read(FILE *in);
object *eval(object *code, object *env);
void print(FILE *out, object *obj, int display);

int check_type(enum obj_type type, object *obj, int err_on_false);

static int is_true(object *obj)
{
	return obj != false;
}

object *make_int(int value);
int obj2int(object *obj);

object *make_bool(int value);
int obj2bool(object *obj);

object *make_char(char c);
char obj2char(object *obj);

object *make_str(char *str);
char *obj2str(object *obj);

object *cons(object *car, object *cdr);
object *car(object *obj);
object *cdr(object *obj);
void set_car(object *pair, object *new);
void set_cdr(object *pair, object *new);

object *make_symbol(char *name);
char *sym2str(object *obj);
object *get_symbol(char *name);

object *make_prim_fun(prim_proc fun);
prim_proc obj2prim_proc(object *obj);

object *make_lambda(object *args, object *code, object *env);
object *lambda_code(object *obj);
object *lambda_args(object *obj);

/*both of these should never be called*/
object *apply_proc(object *);
object *eval_proc(object *);


object *maybe_add_begin(object *code);

void init_enviroment(object *env);


void eval_err(char *msg, object *code);

void define_var(object *var, object *val, object *env);
void set_var(object *var, object *val, object *env);
object *get_var(object *var, object *env);

object *cond2nested_if(object *cond);
object *let2lambda(object *let);
object *and2nested_if(object *and);
object *or2nested_if(object *or);

#endif /*include guard*/