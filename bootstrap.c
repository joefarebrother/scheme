/* 
 * A quick and dirty scheme interpreter for bootstrapping the 
 * compiler for the first time. Has pairs, lambdas, strings, 
 * integers, characters, symbols, and IO, but no vectors or macros 
 * or continuations as they are not needed by the compiler. 
 * Includes a very simple reference-counting GC that will leak 
 * memory on circular structures, which is ok because only one 
 * (the global enviroment) is used, which is permanent anyway. Based on
 * http://michaux.ca/articles/scheme-from-scratch-introduction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cxrs.h"

/*
 * DATA
 */

typedef enum obj_type {
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
		struct object (**prim)(struct object *args);
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
object *eof;
object *empty_list;
object *std_input;
object *std_output;
object *std_error;
object *symbol_table;
object *global_enviroment;


int check_type(enum obj_type type, object *obj, int err_on_false)
{
	int result = (type == obj->type);
	if (!result && err_on_false){
		fprintf(stderr, "Type error.\n");
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

void decrement_refs(object *obj)
{
	if(--(obj->refs) || obj == true || obj == false || 
		 obj == empty_list || obj == eof || obj == std_input ||
		 obj == std_output || obj == std_error)
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
		fclose(obj->data.file);
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
	decrement_refs(car(pair));
	pair->data.pair.car = new;
	new->refs++;
}

void set_cdr(object *pair, object *new)
{
	check_type(scm_pair, pair, 1);
	decrement_refs(cdr(pair));
	pair->data.pair.cdr = new;
	new->refs++;
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

	std_output = alloc_obj();
	std_output->type = scm_file;
	std_output->data.file = stderr;

	eof = alloc_obj();
	eof->type = scm_eof;

	symbol_table = empty_list;

	global_enviroment = cons(empty_list, empty_list);
}

/*
 * Read
 */

int is_delimiter(int c) /*int not char because it might be EOF */
{
	return isspace(c) || c == EOF ||
		   c == '('   || c == ')' ||
		   c == '"'   || c == ';' ||
		   c == '\'';
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

object *read(FILE *in);

object *read_list(FILE *in){
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
void print(FILE *out, object *obj, int display);

void eval_err(char *msg, object *code)
{
	fprintf(stderr, "Evaluation error: %s ", msg);
	print(stderr, code, 0);
	fputc('\n', stderr);
	exit(1);
}

void define_var(object *var, object *val, object *env)
{
	set_car(env, cons(cons(var, val), car(env)));
}

object *find_var_binding(object *var, object *env)
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

int self_evaluating(object *code)
{
#define check(x) check_type(scm_ ## x, code, 0)
 	return check(int) || check(str) || check(char) || check(eof) || check(bool);
#undef check
}


object *eval(object *code, object *env);

object *eval_define(object *code, object *env)
{
	if (!check_type(scm_pair, cdr(code), 0))
			eval_err("bad DEFINE form:", code);

	if(check_type(scm_symbol, cadr(code), 0)) {
		if(!(cddr(code) == empty_list || 
				(check_type(scm_pair, cddr(code), 0) 
				&& cdddr(code) == empty_list)))
			eval_err("bad DEFINE form:", code);

		define_var(cadr(code), 
			(cddr(code) == empty_list ? false : eval(caddr(code), env)), env);
		return cadr(code);
	} 
		
	/*other form of define to be implemented later */	
}

object *eval_list(object *code, object *env)
{
	object *arg;
	if (car(code) == get_symbol("QUOTE")){
		if (!(check_type(scm_pair, cdr(code), 0) && (cddr(code) == empty_list)))
			eval_err("bad QUOTE form:", code);

		return cadr(code);
	}

	else if (car(code) == get_symbol("DEFINE"))
		return eval_define(code, env);

	else if (car(code) == get_symbol("SET!")){
		if(!(check_type(scm_pair, cdr(code), 0) && check_type(scm_pair, cddr(code), 0)
			&& check_type(scm_symbol, cadr(code), 0) && cdddr(code) == empty_list))
			eval_err("bad SET! form:", code);

		arg = eval(caddr(code), env);
		set_var(cadr(code), arg, env);
		return arg;
	}

	else eval_err("can't evaluate", code);
}

object *eval(object *code, object *env)
{
	if(self_evaluating(code))
		return code;
	else if(check_type(scm_pair, code, 0))
		return eval_list(code, env);
	else if(check_type(scm_symbol, code, 0))
		return get_var(code, env);

	else eval_err("can't evaluate", code);
}

/*
 * Print
 */


void print_list(FILE *out, object *list, int display)
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
	printf("Welcome to bootstrap scheme. \n%s",
		  "Press ctrl-c to exit. \n");

	init_constants();

	while(1){
		printf("> ");
		print(stdout, eval(read(stdin), global_enviroment), argc - 1);
		printf("\n");
	}
}