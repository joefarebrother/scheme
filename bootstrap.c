/* 
 * A quick and dirty scheme interpreter for bootstrapping the 
 * compiler for the first time. Has pairs, lambdas, strings, 
 * integers, characters, symbols, and IO, but no vectors or macros 
 * or continuations as they are not needed by the compiler. 
 * Includes a very simple reference-counting GC that will leak 
 * some memory, but it only runs for a short time so it's OK.
 * Based on
 * http://michaux.ca/articles/scheme-from-scratch-introduction.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bootstrap.h"

/*
 * DATA
 */


struct object {
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
		prim_proc prim;
		struct {
			struct object *env;
			struct object *args;
			struct object *code;
		} lambda;
		struct {
			int direction;
			FILE *handle;
		} port;
	} data;
};

static object *symbol_table;

static char *type_name(enum obj_type type)
{
	switch(type){
	case scm_bool:
		return "a boolean";
	case scm_empty_list:
		return "the empty list";
	case scm_eof:
		return "the end of file object";
	case scm_char:
		return "a character";
	case scm_int: 
		return "a number";
	case scm_pair:
		return "a pair";
	case scm_symbol:
		return "a symbol";
	case scm_prim_fun:
	case scm_lambda:
		return "a function";
	case scm_str:
		return "a string";
	case scm_file:
		return "a port";
	default:
		return "unknown"; /* this shouldn't happen */
	}
}

int check_type(enum obj_type type, object *obj, int err_on_false)
{
	int result = (type == obj->type);
	if (!result && err_on_false){
		fprintf(stderr, "Type error: expecting %s, got %s.\n", 
			type_name(type), type_name(obj->type));
		exit(1);
	}
	return result;
}


static object *alloc_obj(void)
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

static void decrement_refs(object *obj)
{
	if(--(obj->refs) || obj == true || obj == false || 
		 obj == empty_list || obj == eof)
		return;

	switch(obj->type){
	case scm_pair:
		decrement_refs(obj->data.pair.car);
		decrement_refs(obj->data.pair.cdr);
		break;
	case scm_str:
	case scm_symbol:
		free(obj->data.str);
		break;
	case scm_lambda:
		decrement_refs(obj->data.lambda.env);
		decrement_refs(obj->data.lambda.args);
		decrement_refs(obj->data.lambda.code);
		break;
	case scm_file:
		fclose(obj->data.port.handle);
		break;
	/*no default branch necassary */
	}
	free(obj);
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
	return obj == true ? 1 : 0;
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
	obj->data.str = strdup(str);
	if (obj->data.str == NULL){
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	}
	return obj;
}

char *obj2str(object *obj)
{
	check_type(scm_str, obj, 1);
	return obj->data.str;
}

object *cons(object *car, object *cdr)
{
	object *obj = alloc_obj();
	obj->type = scm_pair;
	obj->data.pair.car = car;
	obj->data.pair.cdr = cdr;
	car->refs++;
	cdr->refs++;
	return obj;
}

object *car(object *obj)
{
	check_type(scm_pair, obj, 1);
	return obj->data.pair.car;
}

object *cdr(object *obj)
{
	check_type(scm_pair, obj, 1);
	return obj->data.pair.cdr;
}

void set_car(object *pair, object *new)
{
	check_type(scm_pair, pair, 1);
	new->refs++;
	decrement_refs(car(pair));
	pair->data.pair.car = new;
}

void set_cdr(object *pair, object *new)
{
	check_type(scm_pair, pair, 1);
	new->refs++;
	decrement_refs(cdr(pair));
	pair->data.pair.cdr = new;
}

object *make_symbol(char *name)
{
	object *obj = make_str(name);
	obj->type = scm_symbol;
	return obj;
}

char *sym2str(object *obj)
{
	check_type(scm_symbol, obj, 1);
	return obj->data.str;
}

object *get_symbol(char *name)
{
	/* 
	 * This is a SLOW algorithm (O(n))
	 * but it's the simplest one and 
	 * speed isn't important here so
	 * it's ok.
	 */

	object *sym, *table;
	for(table = symbol_table; table != empty_list; table = cdr(table))
		if(!strcmp(name, sym2str(car(table))))
			return car(table);

	sym = make_symbol(name);
	symbol_table = cons(sym, symbol_table);
	return sym;
}

object *make_prim_fun(prim_proc fun)
{
	object *obj = alloc_obj();
	obj->type = scm_prim_fun;
	obj->data.prim = fun;
}
prim_proc obj2prim_proc(object *obj)
{
	check_type(scm_prim_fun, obj, 1);
	return obj->data.prim;
}

object *make_lambda(object *args, object *code, object *env)
{
	object *obj = alloc_obj();
	obj->type = scm_lambda;
	obj->data.lambda.args = args;
	obj->data.lambda.code = code;
	obj->data.lambda.env = env;
	args->refs++;
	code->refs++;
	env->refs++;
	return obj;
}

object *lambda_code(object *obj)
{
	check_type(scm_lambda, obj, 1);
	return obj->data.lambda.code;
}

object *lambda_args(object *obj)
{
	check_type(scm_lambda, obj, 1);
	return obj->data.lambda.args;
}

object *lambda_env(object *obj)
{
	check_type(scm_lambda, obj, 1);
	return obj->data.lambda.env;
}

object *make_port(FILE *handle, int direction)
{
	object *obj = alloc_obj();
	obj->type = scm_file;
	obj->data.port.handle = handle;
	obj->data.port.direction = direction;
	return obj;
}

/*1 is input, 0 is output*/
int port_direction(object *obj)
{
	check_type(scm_file, obj, 1);
	return obj->data.port.direction;
}

FILE *port_handle(object *obj)
{
	check_type(scm_file, obj, 1);
	return obj->data.port.handle;
}

void set_port_handle_to_null(object *obj)
{
	check_type(scm_file, obj, 1);
	obj->data.port.handle = NULL;
}

static void init_constants(void)
{
	true = alloc_obj();
	true->type = scm_bool;

	false = alloc_obj();
	false->type = scm_bool;

	empty_list = alloc_obj();
	empty_list->type = scm_empty_list;

	eof = alloc_obj();
	eof->type = scm_eof;

	symbol_table = empty_list;

	global_enviroment = cons(empty_list, empty_list);
}

/*
 * Read
 */

static int is_delimiter(int c) /*int not char because it might be EOF */
{
	return isspace(c) || c == EOF ||
		   c == '('   || c == ')' ||
		   c == '"'   || c == ';' ||
		   c == '\'';
}

static char peek(FILE *in){
	char c = getc(in);
	ungetc(c, in);
	return c;
}


static void eat_ws(FILE *in)
{
	int c;
	while((c = getc(in)) != EOF){
		if (isspace(c)) continue;
		else if (c == ';'){  /* comment - ignore */
			while((c = getc(in)) != '\n' && c != EOF);
			continue;
		}
		ungetc(c, in);
		break;
	}
}

static void eat_expected_str(FILE *in, char *str)
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

static void expect_delim(FILE *in)
{
	if(!is_delimiter(peek(in))){
		fprintf(stderr, "Unexpected character: expecting a delimiter, got %c.\n", peek(in));
		exit(1);
	}
}

static object *read_char(FILE *in) 
{
    int c;

    c = getc(in);
    switch (c) {
    case EOF:
        fprintf(stderr, "Unexpected end of file: Incomplete character literal.\n");
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

static object *read_list(FILE *in){
	object *car, *cdr;
	int c;
	eat_ws(in);

	c = getc(in);
	if (c == ')') 
		return empty_list;
	if (c == EOF){
		fprintf(stderr, "Unexpected end of file: unclosed list.\n");
		exit(1);
	}
	ungetc(c, in);

	car = read(in);
	eat_ws(in);

	if(peek(in) == '.'){ /* improper list */
		getc(in);
		if(!is_delimiter(c = peek(in))){
			fprintf(stderr, "Bad list: expecting delimiter after dot, got %c.\n", c);
			exit(1);
		}
		cdr = read(in);

		eat_ws(in);
		if (c = getc(in) != ')'){
			fprintf(stderr, "Bad list: expecting ), got %c.\n", c);
			exit(1);
		}
	}
	else cdr = read_list(in);

	return cons(car, cdr);
}

#define BUF_MAX 1024
object *read(FILE *in)
{
	int c; 

	eat_ws(in);

	c = getc(in);

	if (c == EOF) return eof;
	else if (isdigit(c) || (c == '-' && isdigit(peek(in))))
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
			fprintf(stderr, "Expecting delimiter after number, got %c.\n", c);
			exit(1);
		}

		ungetc(c, in);
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
		case '<':
			fprintf(stderr, "Unreadable object in input stream.\n");
			exit(1);
		default:
			fprintf(stderr, "Bad input. Expecting t, f, or \\, got %c.\n", c);
			exit(1);
		}
	}
	else if (c == '"')
	{
		/* read a string */
		char buf[BUF_MAX];
		int len = 0;

		while ((c = getc(in)) != '"'){
			if (c == EOF){
				fprintf(stderr, "Unexpected end of file: Non terminated string literal.\n");
				exit(1);
			}

			if (c == '\\'){
				c = getc(in);
				if (c == 'n') c = '\n';
				if (c == 't') c = '\t';
			}

			buf[len++] = c;
			if (len == BUF_MAX){
				fprintf(stderr, "String too long. Maximum length is %d.\n", BUF_MAX);
				exit(1);
			}
		}
		buf[len] = '\0';
		return make_str(buf);
	}
	else if (c == '('){
		/* read a list */
		return read_list(in);
	}
	else if (c == '\''){
		/* quote */
		return cons(get_symbol("QUOTE"), 
			cons(read(in), empty_list));
	}

	else if (!is_delimiter(c)) {
		/*read a symbol*/
		char buf[BUF_MAX];
		int len = 0;
		ungetc(c, in);

		while(!is_delimiter(c = getc(in))){
			buf[len++] = toupper(c);
			if (len == BUF_MAX) {
				fprintf(stderr, "Symbol too long. Makimum length is %d.\n", BUF_MAX);
				exit(1);
			}
		}
		ungetc(c, in);
		buf[len] = '\0';
		return get_symbol(buf);
	}

	else {
		fprintf(stderr, "Bad input: Unexpected %c.\n", c);
		exit(1);
	}
}


/*
 * Eval - based on SICP
 */

void eval_err(char *msg, object *code)
{
	fprintf(stderr, "Evaluation error: %s ", msg);
	print(stderr, code, 0);
	fputc('\n', stderr);
	exit(1);
}

/* 
 * returns 1 if code's length is between min and max inclusive, 0 otherwise 
 * min or max of -1 indicate no minimum/maximum 
 */
static int check_length_between(int min, int max, object *code){
	while(1){
		if (min < 0 && max < 0) return 1;
		if (code == empty_list && min <= 0) return 1;

		if(max == 0) return 0;
		if(!check_type(scm_pair, code, 0)) return 0;

		code = cdr(code);
		--min;
		--max;
	}
}

void define_var(object *var, object *val, object *env)
{
	set_car(env, cons(cons(var, val), car(env)));
}

static object *find_var_binding(object *var, object *env)
{
	object *frame;
	for(; env != empty_list; env = cdr(env))
		for(frame = car(env); frame != empty_list; frame = cdr(frame))
			if(caar(frame) == var)
				return car(frame);
	eval_err("unbound variable", var);
}

void set_var(object *var, object *val, object *env)
{
	set_cdr(find_var_binding(var, env), val);
}

object *get_var(object *var, object *env)
{
	return cdr(find_var_binding(var, env));
}

static object *make_frame(object *vars, object *vals)
{
	object *sofar = empty_list;
	while(vals != empty_list){
		if (vars == empty_list)
			eval_err("Too many arguments to a function, excessive arguments are:", vals);

		if(check_type(scm_symbol, vars, 0))
			return cons(cons(vars, vals), sofar);
		sofar = cons(cons(car(vars), car(vals)), sofar);
		vars = cdr(vars);
		vals = cdr(vals);
	}
	if(vars != empty_list)
		eval_err("Not enough arguments to a function, these variables had no value:", vars);

	return sofar;
}

object *extend_enviroment(object *vars, object *vals, object *env)
{
	return cons(make_frame(vars, vals), env);
}

static int self_evaluating(object *code)
{
#define check(x) check_type(scm_ ## x, code, 0)
 	return check(int) || check(str) || check(char) || check(eof) || check(bool);
#undef check
}

object *maybe_add_begin(object *code)
{
	if (cdr(code) == empty_list) return car(code);
	return cons(get_symbol("BEGIN"), code);
}

static object *eval_define(object *code, object *env)
{
	if (!check_length_between(2, -1, code))
			eval_err("bad DEFINE form:", code);

	if(check_type(scm_symbol, cadr(code), 0)) {
		if(!check_length_between(2, 3, code))
			eval_err("bad DEFINE form:", code);

		define_var(cadr(code), 
			(cddr(code) == empty_list ? false : eval(caddr(code), env)), env);
		return cadr(code);
	} 
		
	if(check_type(scm_pair, cadr(code), 0)){
		if(!check_length_between(3, -1, code)) 
			eval_err("bad DEFINE form:", code);

		define_var(caadr(code), make_lambda(cdadr(code), maybe_add_begin(cddr(code)), env), env);
		return caadr(code);
	}

	eval_err("bad DEFINE form:", code);
}

static object *eval_each(object *exprs, object *env)
{
	if(exprs == empty_list)
		return empty_list;
	else
		return cons(eval(car(exprs), env), eval_each(cdr(exprs), env));
}


object *eval(object *code, object *env)
{

tailcall:
#define tail(x) do{code = (x); goto tailcall;} while(0)

	if(self_evaluating(code))
		return code;
	else if(check_type(scm_symbol, code, 0))
		return get_var(code, env);

	else if(check_type(scm_pair, code, 0)){
#define starts_with(s) (car(code) == get_symbol(#s))

		if starts_with(QUOTE){
			if (!check_length_between(2, 2, code))
				eval_err("bad QUOTE form:", code);

			return cadr(code);
		}

		else if starts_with(DEFINE)
			return eval_define(code, env);

		else if starts_with(SET!){
			if(!check_length_between(3, 3, code) || !check_type(scm_symbol, cadr(code), 0))
				eval_err("bad SET! form:", code);

			set_var(cadr(code), eval(caddr(code), env), env);
			return get_symbol("OK");
		}

		else if starts_with(IF){
			if(!check_length_between(3, 4, code))
				eval_err("bad IF form:", code);

			tail(
				is_true(eval(cadr(code), env)) ? caddr(code) : /* cond in C! */
				cdddr(code) == empty_list      ? false       : /*undefined when no else branch*/
				/* else */   cadddr(code));
		}

		else if starts_with(LAMBDA){
			if(!check_length_between(3, -1, code)) 
				eval_err("bad LAMBDA form:", code);

			return make_lambda(cadr(code), maybe_add_begin(cddr(code)), env);
		}

		else if starts_with(BEGIN){
			if(!check_length_between(2, -1, code)) 
				eval_err("bad BEGIN form:", code);

			for(code = cdr(code); cdr(code) != empty_list; code = cdr(code))
				eval(car(code), env);

			tail(car(code));
		}

		/*syntaxes*/

		else if starts_with(COND)
			tail(cond2nested_if(code));

		else if starts_with(LET)
			tail(let2lambda(code));

		else if starts_with(AND)
			tail(and2nested_if(code));

		else if starts_with(OR)
			tail(or2nested_if(code));

		/*more stuff can go here*/

		else{	
			/*it's a call*/
			object *proc = eval(car(code), env);
			object *args = eval_each(cdr(code), env);
retry_apply:
			if(check_type(scm_prim_fun, proc, 0)){
				if(obj2prim_proc(proc) == apply_proc){/*apply should never be called    */
					proc = car(args);                 /*directly because of tail call   */
					args = cadr(args);                /*requirements. The implementation*/
					goto retry_apply;                 /*in bootstrap-prims.c signals an */
				}                                     /*error if it is.                 */

				if(obj2prim_proc(proc) == eval_proc){ /*same with eval*/
					env = cadr(args);
					tail(car(args));
				}
				
				return (obj2prim_proc(proc))(args);
			}
			if(!check_type(scm_lambda, proc, 0))
				eval_err("not a function:", proc);
	
			env = extend_enviroment(lambda_args(proc), args, lambda_env(proc));
			tail(lambda_code(proc));
		}
	}

	else eval_err("can't evaluate", code);
}

/*
 * Print
 */


static void print_list(FILE *out, object *list, int display)
{
	fputc('(', out);
		print(out, car(list), display);
	for (list = cdr(list); check_type(scm_pair, list, 0) ; list = cdr(list))
	{
		fputc(' ', out);
		print(out, car(list), display);
	}

	if(list == empty_list)
		fputc(')', out);
	else {
		fprintf(out, " . ");
		print(out, list, display);
		fputc(')', out);
	}
}

void print(FILE *out, object *obj, int display)
{
	switch(obj->type) {
	case scm_int:
		fprintf(out, "%d", obj2int(obj));
		break;

	case scm_bool:
		fprintf(out, obj2bool(obj) ? "#t" : "#f");
		break;

	case scm_eof:
		fprintf(out, "#<eof object>");
		break;

	case scm_empty_list:
		fprintf(out, "()");
		break;

	case scm_pair:
		print_list(out, obj, display);
		break;

	case scm_symbol:
		fprintf(out, "%s", sym2str(obj));
		break;

	case scm_prim_fun:
		fprintf(out, "#<primitive procedure>");
		break;

	case scm_lambda:
		fprintf(out, "#<procedure>");
		break;

	case scm_file:
		fprintf(out, "#<%s port>", port_direction(obj) ? "Input" : "Output");
		break;

	case scm_char:
		if (display) fputc(obj2char(obj), out);
		else {
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
			}
		break;

	case scm_str:
		if (display) fprintf(out, "%s", obj2str(obj));
		else {
			char *ptr = obj2str(obj);

			fputc('"', out);
			while (*ptr != '\0'){
				char c = *ptr++;
				switch (c){
				case '\n':
					fprintf(out, "\\n");
					break;
				case '\t':
					fprintf(out, "\\t");
					break;
				case '\\':
					fprintf(out, "\\\\");
					break;
				case '"':
					fprintf(out, "\\\"");
				default:
					fputc(c, out);
				}
			}
			fputc('"', out);
		}
		break;

	default:
		fprintf(stderr, "Unknown data type in write: %d.\n", obj->type);
		exit(1);
	}
}


/*
 * REPL
 */
int main(int argc, const char **argv)
{
	printf("Welcome to bootstrap scheme. \n"
		  "Press ctrl-c or type (exit) to exit. \n");

	init_constants();
	init_enviroment(global_enviroment);

	while(1){
		printf("> ");
		print(stdout, eval(read(stdin), global_enviroment), argc - 1);
		printf("\n");
	}
}