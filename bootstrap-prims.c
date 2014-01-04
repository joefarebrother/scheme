/*
 * Primitive procedures for the bootstrap scheme interpreter
 */

#include <stdlib.h>
#include <string.h>
#include "bootstrap.h"

/*type_procedures*/
#define DEF_TYPE_PROC(type) object *is_ ## type ## _proc(object *args) \
 { \
  	return make_bool(check_type(scm_ ## type, car(args), 0)); \
 }

DEF_TYPE_PROC(bool);
DEF_TYPE_PROC(empty_list);
DEF_TYPE_PROC(char);
DEF_TYPE_PROC(int);
DEF_TYPE_PROC(pair);
DEF_TYPE_PROC(symbol);
DEF_TYPE_PROC(prim_fun);
DEF_TYPE_PROC(str);

/*lists*/
object *car_proc(object *args){return caar(args);}
object *cdr_proc(object *args){return cdar(args);}

object *cons_proc(object *args)
{
	set_cdr(args, cadr(args)); /*args is allocated fresh each time!*/
	return args;
}

/*arithmetic*/
object *add_proc(object *args)
{
	int sum = 0;
	for(; args != empty_list; args = cdr(args))
		sum += obj2int(car(args));

	return make_int(sum);
}

object *mul_proc(object *args)
{
	int prod;
	for(prod = 1; args != empty_list; args = cdr(args))
		prod *= obj2int(car(args));

	return make_int(prod);
}


/*initialise*/
#define SYMBUF_SIZE 50
static object *to_sym(char *str)
{
	char buf[SYMBUF_SIZE];
	int len;
	for(len = 0; *str != '\0'; str++, len++){
		if(len == SYMBUF_SIZE){
			fprintf(stderr, "Internal error: buffer exceeded during initialisation.\n");
			exit(1);
		}
		buf[len] = *str == '_' ? '-' : toupper(*str);
	}
	buf[len] = '\0';
	return get_symbol(buf);
}


#define DEFPROC(n, f) \
	define_var(to_sym(#n), make_prim_fun(f ## _proc), global_enviroment)
#define DEFPROC1(n) DEFPROC(n, n)
void init_global_enviroment(void)
{
	DEFPROC(+, add);
	DEFPROC(*, mul);
	DEFPROC1(car);
	DEFPROC1(cdr);
	DEFPROC(boolean?, is_bool);
	DEFPROC(null?, is_empty_list);
	DEFPROC(char?, is_char);
	DEFPROC(integer?, is_int);
	DEFPROC(pair?, is_pair);
	DEFPROC(symbol?, is_symbol);
	DEFPROC(procedure?, is_prim_fun);
	DEFPROC(string?, is_str);
}
