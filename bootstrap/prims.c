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
DEF_TYPE_PRED(file);
DEF_TYPE_PRED(eof);
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

static object *number_2string_proc(object *args)
{
	return make_str(int_to_string(obj2int(args)));
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
	return cons(car(args), cadr(args));
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

/*enviroments*/
static object *interaction_enviroment_proc(object *ignore)
{
	return global_enviroment;
}

static object *enviroment_proc(object *ignore)
{
	object *it = cons(empty_list, empty_list);
	init_enviroment(it);
	return it;
}

static object *null_enviroment_proc(object *ignore)
{
	return cons(empty_list, empty_list);
}

/*IO - input*/
static object *open_input_file_proc(object *args)
{
	FILE *in = fopen(obj2str(car(args)), "r");
	if (in == NULL)
		eval_err("Could not open", car(args));

	return make_port(in, 1);
}

static FILE *optional_input_port(object *args)
{
	FILE *in;
	if(args == empty_list) 
		in = stdin;
	else{
		if (!port_direction(car(args)))
			eval_err("Not an input port:", car(args));
		else in = port_handle(car(args));
	}
	if (in == NULL)
		eval_err("File has been closed:", car(args));
	return in;
}

static object *read_char_proc(object *args)
{
	int c = getc(optional_input_port(args));
	if (c == EOF) return eof;
	else return make_char(c);
}

static object *unread_char_proc(object *args)
{
	ungetc(obj2char(car(args)), optional_input_port(cdr(args)));
	return get_symbol("OK");
}

static object *is_input_port_proc(object *args)
{
	return make_bool(check_type(scm_file, car(args), 0) && port_direction(car(args)));
}

static object *close_file_proc(object *args)
{
	if(port_handle(car(args)) == NULL) return get_symbol("ALREADY-CLOSED");
	fclose(port_handle(car(args)));
	set_port_handle_to_null(car(args));
	return get_symbol("OK");
}

static object *read_proc(object *args)
{
	return read(optional_input_port(args));
}

static object *load_proc(object *args)
{
	FILE *in = fopen(obj2str(car(args)), "r");
	object *expr;
	if (in == NULL)
		eval_err("Could not load", car(args));
	while((expr = read(in)) != eof){
		print(stdout, eval(expr, global_enviroment), 1);
		fputc('\n', stdout);
	}
	return get_symbol("PROGRAM-LOADED");
}

/*IO - output*/
static object *open_output_file_proc(object *args)
{
	FILE *out = fopen(obj2str(car(args)), (cdr(args) == empty_list 
										  || cadr(args) == get_symbol("OVERWRITE") ? 
											 "w" : "a"));
	if (out == NULL)
		eval_err("Could not open", car(args));

	return make_port(out, 0);
}

static FILE *optional_output_port(object *args)
{
	FILE *out;
	if(args == empty_list) 
		out = stdout;
	else{
		if (port_direction(car(args)))
			eval_err("Not an output port:", car(args));
		else out = port_handle(car(args));
	}
	if (out == NULL)
		eval_err("File has been closed:", car(args));
	return out;
}

static object *write_char_proc(object *args)
{
	fputc(obj2char(car(args)), optional_output_port(cdr(args)));
	return get_symbol("OK");
}

static object *is_output_port_proc(object *args)
{
	return make_bool(check_type(scm_file, car(args), 0) && !port_direction(car(args)));
}

static object *write_proc(object *args)
{
	print(optional_output_port(cdr(args)), car(args), 0);
	return get_symbol("OK");
}

static object *display_proc(object *args)
{
	print(optional_output_port(cdr(args)), car(args), 1);
	return get_symbol("OK");
}

/*Strings & characters*/
static object *string_append_proc(object *args)
{
	char *str = str_append(obj2str(car(args)), obj2str(cadr(args)));
	object *ret;

	if(str == NULL)
		eval_err("Out of memory", args);

	ret = make_str(str);
	free(str);
	return ret;
}

static object *string_length_proc(object *args)
{
	return make_int(strlen(obj2str(car(args))));
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

static object *gensym_proc(object *args)
{
	static count = 1;
	return make_symbol(str_append("#:G", int_to_string(count++)));
}

object *apply_proc(object *illegal)
{
	fprintf(stderr, "Illegal state: the body of the apply"
		 "primitive procedure should never be excecuted\n");
	exit(1);
}

object *eval_proc(object *illegal)
{
	fprintf(stderr, "Illegal state: the body of the eval"
		 "primitive procedure should never be excecuted\n");
	exit(1);
}

static object *error_proc(object *args)
{
	object *reason;
	fprintf(stderr, "Error in %s: ", sym2str(car(args)));
	for(reason = cdr(args); reason != empty_list; reason = cdr(reason)){
		print(stderr, car(reason), 1);
		fputc(' ', stderr);
	}
	fputc('\n', stderr);
	exit(1);
}

static object *system_proc(object *args)
{
	return(make_int(system(obj2str(car(args)))));
}

/*initialise*/
#define SYMBUF_SIZE 25
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
				   *str == '2' ? '>' : /* a kludge that allows to create conversions (x->y) while using a valid C identifiers; 2 indicates conversion, 
				   						  because of this kludge there are names like string_2number rather than string2number */
				   				 toupper(*str);
	}
	buf[len] = '\0';
	return get_symbol(buf);
}


#define DEFPROC(n, f) \
	define_var(to_sym(#n), make_prim_fun(f ## _proc), env)
#define DEFPROC1(n) DEFPROC(n, n)
void init_enviroment(object *env)
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
	DEFPROC(port?, is_file);
	DEFPROC(eof_object?, is_eof);

	DEFPROC1(integer_2char);
	DEFPROC1(char_2integer);
	DEFPROC1(number_2string);
	DEFPROC1(string_2number);
	DEFPROC1(string_2symbol);
	DEFPROC1(symbol_2string);

	DEFPROC1(interaction_enviroment);
	DEFPROC1(enviroment);
	DEFPROC1(null_enviroment);

	DEFPROC1(open_input_file);
	DEFPROC(close_input_file, close_file);
	DEFPROC1(read_char);
	DEFPROC1(unread_char);
	DEFPROC(input_port?, is_input_port);
	DEFPROC1(read);
	DEFPROC1(load);

	DEFPROC1(open_output_file);
	DEFPROC(close_output_file, close_file);
	DEFPROC1(write_char);
	DEFPROC(output_port?, is_output_port);
	DEFPROC1(write);
	DEFPROC1(display);

	DEFPROC1(string_append);
	DEFPROC1(string_length);

	DEFPROC1(exit);
	DEFPROC(eq?, eq);
	DEFPROC1(apply);
	DEFPROC1(eval);
	DEFPROC1(error);
	DEFPROC1(system);
	DEFPROC1(gensym);
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

	if (cdr(cond) == empty_list) return false; /*undefined*/

	if (caadr(cond) == get_symbol("ELSE")) return maybe_add_begin(cdadr(cond));

	smaller_cond = cons(car(cond), cddr(cond)); /*car(cond) is the symbol 'cond. */

	if (cdadr(cond) == empty_list)
		return list(3, get_symbol("LET"), list(1, list(2, gensym, caadr(cond))),
						list(4, get_symbol("IF"), gensym, 
												  gensym,
												  smaller_cond));

	if (cadadr(cond) == get_symbol("=>"))
		return list(3, get_symbol("LET"), list(1, list(2, gensym, caadr(cond))),
						list(4, get_symbol("IF"), gensym, 
												  list(2, car(cadadr(cond)), gensym),
												  smaller_cond));

	return list(4, get_symbol("IF"), caadr(cond), 
									 maybe_add_begin(cdadr(cond)), 
									 smaller_cond);
}

/*can't use map (defined later) because cxrs are macros */
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
	return list(-1, list(-1, get_symbol("LAMBDA"), let_vars(cadr(let)), 
								NULL, cddr(let)), 
					NULL, let_vals(cadr(let)));
}

object *and2nested_if(object *and)
{
	if (cdr(and) == empty_list)
		return true;
	if (cddr(and) == empty_list)
		return cadr(and);

	return list(4, get_symbol("IF"), cadr(and), 
									 cons(car(and), cddr(and)), 
									 false);
}

object *or2nested_if(object *or)
{
	object *gensym = make_symbol("temp");
	if (cdr(or) == empty_list)
		return false;
	if (cddr(or) == empty_list)
		return cadr(or);

	return list(3, get_symbol("LET"), list(1, list(2, gensym, cadr(or))),
					list(4, get_symbol("IF"), gensym,
											  gensym,
											  cons(car(or), cddr(or))));
}

static object *map(object *f(object *), object *list)
{
	if (list == empty_list) return empty_list;
	else return cons(f(car(list)), map(f, cdr(list)));
}
static object *map2(object *f(object *, object *), object *list1, object *list2)
{
	if (list1 == empty_list) return empty_list;
	else return cons(f(car(list1), car(list2)), map2(f, cdr(list1), cdr(list2)));
}

static object *set_to(object *var, object *val)
{
	return list(3, get_symbol("SET!"), var, val);
}

static object *blank_define(object *var)
{
	return list(2, get_symbol("DEFINE"), var);
}

static object *dummy_binding(object *var)
{
	return list(2, var, false);
}

