/*
 * Primitive procedures for the bootstrap scheme interpreter
 */

#include <stdlib.h>
#include <string.h>
#include "bootstrap.h"
#include <stdarg.h>

/*type predicates*/
#define DEF_TYPE_PRED(type) static object *is_ ## type ## _proc(object *args) \
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

static object *is_proc_proc(object *args) /*a proc that checks if it's arg is a proc, hence proc twice*/
{
	return make_bool(check_type(scm_prim_fun, car(args), 0) ||
		check_type(scm_lambda, car(args), 0));
}

/*conversions*/
static object *integer_2char_proc(object *args)
{
	return make_char(obj2int(car(args)));
}
static object *char_2integer_proc(object *args)
{
	return make_int(obj2char(car(args)));
}

static object *string_2symbol_proc(object *args)
{
	return get_symbol(obj2str(car(args)));
}
static object *symbol_2string_proc(object *args)
{
	return make_str(sym2str(car(args)));
}

#define INT2STR_BUFLEN 12 /* a 32 bit integer is never longer than 12 digits in decimal */
static object *number_2string_proc(object *args)
{
	char buf[INT2STR_BUFLEN];
	snprintf(buf, INT2STR_BUFLEN, "%d", obj2int(car(args)));
	return make_str(buf);
}
static object *string_2number_proc(object *args)
{
	char *str = obj2str(car(args));
	char *end;
	int num = (int)strtol(str, &end, cdr(args) == empty_list ? 10 : obj2int(cadr(args)));
	if(end == str) return false;
	else return make_int(num);
}

/*lists*/
static object *car_proc(object *args){return caar(args);}
static object *cdr_proc(object *args){return cdar(args);}

static object *cons_proc(object *args)
{
	set_cdr(args, cadr(args)); /*args is allocated fresh each time!*/
	return args;
}

static object *set_car_proc(object *args)
{
	set_car(car(args), cadr(args));
	return(get_symbol("OK"));
}

static object *set_cdr_proc(object *args)
{
	set_cdr(car(args), cadr(args));
	return(get_symbol("OK"));
}

/*arithmetic*/
static object *add_proc(object *args)
{
	int sum = 0;
	for(; args != empty_list; args = cdr(args))
		sum += obj2int(car(args));

	return make_int(sum);
}

static object *mul_proc(object *args)
{
	int prod;
	for(prod = 1; args != empty_list; args = cdr(args))
		prod *= obj2int(car(args));

	return make_int(prod);
}

static object *sub_proc(object *args)
{
	int sofar;
	if(cdr(args) == empty_list)
		return make_int(-obj2int(car(args)));

	for(sofar = obj2int(car(args)), args = cdr(args); args != empty_list; args = cdr(args))
		sofar -= obj2int(car(args));

	return make_int(sofar);
}

static object *quotient_proc(object *args)
{
	return make_int(obj2int(car(args)) / obj2int(cadr(args)));
}
static object *remainder_proc(object *args)
{
	return make_int(obj2int(car(args)) % obj2int(cadr(args)));
}

static object *equals_proc(object *args)
{
	int first = obj2int(car(args));
	for(args = cdr(args); args != empty_list; args = cdr(args))
		if(obj2int(car(args)) != first)
			return false;

	return true;
}

static object *greater_than_proc(object *args)
{
	int big = obj2int(car(args));
	for(args = cdr(args); args != empty_list; args = cdr(args)){
		if(big > obj2int(car(args)))
			big = obj2int(car(args));
		else return false;
	}
	return true;
}

static object *less_than_proc(object *args)
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
static object *exit_proc(object *args)
{
	exit(args == empty_list ? 0 : obj2int(car(args)));
}

static object *eq_proc(object *args)
{
	return make_bool(car(args) == cadr(args));
}

static object *apply_proc(object *args)
{
	return apply(car(args), cadr(args));
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
	DEFPROC1(apply);
}

/*syntaxes*/

static object *build_list(int length, va_list args)
{
	object *car;
	if (length == 0)  return empty_list;
	car = va_arg(args, object *);
	if (car == NULL) /*NULL acts as a dot. Length doesn't matter if there's a NULL.*/
	return va_arg(args, object *);

	return cons(car, build_list(length-1, args));
}

static object *list(int length, ...)
{
	va_list args;
	object *result;
	va_start(args, length);
	result = build_list(length, args);
	va_end(args);
	return result;
}

object *cond2nested_if(object *cond)
{
	object *smaller_cond, *gensym = make_symbol("temp"); /*make symbol returns a new one*/
	if (cdr(cond) == empty_list) return false;
	if (caadr(cond) == get_symbol("ELSE")) return maybe_add_begin(cdadr(cond));
	smaller_cond = cons(get_symbol("COND"), cddr(cond));
	if (cdadr(cond) == empty_list)
		return list(2, list(3, get_symbol("LAMBDA"), list(1, gensym),
			list(4, get_symbol("IF"), gensym, gensym, smaller_cond)), caadr(cond));

	if (cadadr(cond) == get_symbol("=>"))
		return list(2, list(3, get_symbol("LAMBDA"), list(1, gensym),
			list(4, get_symbol("IF"), gensym, list(2, car(cddadr(cond)), gensym), smaller_cond)), caadr(cond));

	return list(4, get_symbol("IF"), caadr(cond), maybe_add_begin(cdadr(cond)), smaller_cond);
}

#define DEF_BINDING_SELECTOR(name, cxr)          \
static object *name(object *bindings)             \
{                                                  \
	if(bindings == empty_list) return empty_list;   \
	return(cons(cxr(bindings), name(cdr(bindings))));\
}

DEF_BINDING_SELECTOR(let_vars, caar);
DEF_BINDING_SELECTOR(let_vals, cadar);

object *let2lambda(object *let)
{
	return list(-1, list(-1, get_symbol("LAMBDA"), let_vars(cadr(let)), NULL, cddr(let)), 
		NULL, let_vals(cadr(let)));
}