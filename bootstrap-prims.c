/*
 * Primitive procedures for the bootstrap scheme interpreter
 */

#include <stdlib.h>
#include <string.h>
#include "bootstrap.h"

/*type predicates*/
#define DEF_TYPE_PRED(type) object *is_ ## type ## _proc(object *args) \
 { \
  	return make_bool(check_type(scm_ ## type, car(args), 0)); \
 }

DEF_TYPE_PRED(bool);
DEF_TYPE_PRED(char);
DEF_TYPE_PRED(int);
DEF_TYPE_PRED(pair);
DEF_TYPE_PRED(symbol);
/*
DEF_TYPE_PRED(prim_fun);
DEF_TYPE_PRED(lambda);
*/
DEF_TYPE_PRED(str);

object *is_proc_proc(object *args) /*a proc that checks if it's arg is a proc, hence proc twice*/
{
	return make_bool(check_type(scm_prim_fun, car(args), 0) ||
		check_type(scm_lambda, car(args), 0));
}

/*conversions*/
object *integer_2char_proc(object *args)
{
	return make_char(obj2int(car(args)));
}
object *char_2integer_proc(object *args)
{
	return make_int(obj2char(car(args)));
}

object *string_2symbol_proc(object *args)
{
	return get_symbol(obj2str(car(args)));
}
object *symbol_2string_proc(object *args)
{
	return make_str(sym2str(car(args)));
}

#define INT2STR_BUFLEN 12 /* a 32 bit integer is never longer than 12 digits in decimal */
object *number_2string_proc(object *args)
{
	char buf[INT2STR_BUFLEN];
	snprintf(buf, INT2STR_BUFLEN, "%d", obj2int(car(args)));
	return make_str(buf);
}
object *string_2number_proc(object *args)
{
	char *str = obj2str(car(args));
	char *end;
	int num = (int)strtol(str, &end, cdr(args) == empty_list ? 10 : obj2int(cadr(args)));
	if(end == str) return false;
	else return make_int(num);
}

/*lists*/
object *car_proc(object *args){return caar(args);}
object *cdr_proc(object *args){return cdar(args);}

object *cons_proc(object *args)
{
	set_cdr(args, cadr(args)); /*args is allocated fresh each time!*/
	return args;
}

object *set_car_proc(object *args)
{
	set_car(car(args), cadr(args));
	return(get_symbol("OK"));
}

object *set_cdr_proc(object *args)
{
	set_cdr(car(args), cadr(args));
	return(get_symbol("OK"));
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

object *sub_proc(object *args)
{
	int sofar;
	if(cdr(args) == empty_list)
		return make_int(-obj2int(car(args)));

	for(sofar = obj2int(car(args)), args = cdr(args); args != empty_list; args = cdr(args))
		sofar -= obj2int(car(args));

	return make_int(sofar);
}

object *quotient_proc(object *args)
{
	return make_int(obj2int(car(args)) / obj2int(cadr(args)));
}
object *remainder_proc(object *args)
{
	return make_int(obj2int(car(args)) % obj2int(cadr(args)));
}

object *equals_proc(object *args)
{
	int first = obj2int(car(args));
	for(args = cdr(args); args != empty_list; args = cdr(args))
		if(obj2int(car(args)) != first)
			return false;

	return true;
}

object *greater_than_proc(object *args)
{
	int big = obj2int(car(args));
	for(args = cdr(args); args != empty_list; args = cdr(args)){
		if(big > obj2int(car(args)))
			big = obj2int(car(args));
		else return false;
	}
	return true;
}

object *less_than_proc(object *args)
{
	int small = obj2int(car(args));
	for(args = cdr(args); args != empty_list; args = cdr(args)){
		if(small < obj2int(car(args)))
			small = obj2int(car(args));
		else return false;
	}
	return true;
}

/*misc*/
object *exit_proc(object *args)
{
	exit(args == empty_list ? 0 : obj2int(car(args)));
}

object *eq_proc(object *args)
{
	return make_bool(car(args) == cadr(args));
}

/*initialise*/
#define SYMBUF_SIZE 20
static object *to_sym(char *str)
{
	char buf[SYMBUF_SIZE];
	int len;
	for(len = 0; *str != '\0'; str++, len++){
		if(len == SYMBUF_SIZE){
			fprintf(stderr, "Internal error: buffer exceeded during initialisation.\n");
			exit(1);
		}
		buf[len] = *str == '_' ? '-' : 
				   *str == '2' ? '>' : /* conversions: integer_2char becomes integer->char */
				   				 toupper(*str);
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
	DEFPROC(-, sub);
	DEFPROC1(quotient);
	DEFPROC1(remainder);
	DEFPROC(=, equals);
	DEFPROC(>, greater_than);
	DEFPROC(<, less_than);

	DEFPROC1(car);
	DEFPROC1(cdr);
	DEFPROC1(cons);
	DEFPROC(set_car!, set_car);
	DEFPROC(set_cdr!, set_cdr);

	DEFPROC(boolean?, is_bool);
	DEFPROC(char?, is_char);
	DEFPROC(integer?, is_int);
	DEFPROC(pair?, is_pair);
	DEFPROC(symbol?, is_symbol);
	DEFPROC(procedure?, is_proc);
	DEFPROC(string?, is_str);

	DEFPROC1(integer_2char);
	DEFPROC1(char_2integer);
	DEFPROC1(number_2string);
	DEFPROC1(string_2number);
	DEFPROC1(string_2symbol);
	DEFPROC1(symbol_2string);

	DEFPROC1(exit);
	DEFPROC(eq?, eq);
}
