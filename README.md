scheme
======

(what will be) A scheme compiler

The bootstrap directory contains the following:
bootstrap.c is a bootstrap interpreter for scheme, intended for bootstraping compile/compile.scm (currently non-existant because git doeesn't track empty directories).
prims.c contains the primitive procedures for the bootstrapper
lib.scm implements a standard library for the bootstrapper to run.

To test, type at a terminal:

```shell
$ make
$ ./bootstrap/bootstrap
&gt; (load "bootstrap/lib.scm")
```

prims.c currently defines:

- char->integer
- integer->char
- number->string
- string->number
- symbol->string
- string->symbol
- boolean?
- char?
- integer?
- pair?
- symbol?
- procedure?
- string?
- port?
- +
- -
- *
- quotient
- remainder
- =
- <
- >
- cons
- car
- cdr
- set-car!
- set-cdr!
- eq?
- string-append
- apply
- eval
- exit
- interaction-enviroment
- enviroment
- null-enviroment
- open-input-file
- read-char
- unread-char (non-standard) - (unread-char char port) pushes a character back to an input port
- eof-object?
- close-input-file
- read
- load
- open-output-file (takes a non-standard optional second argument a symbol indicating what to di if it already exists: overwrite or append. Default is overwrite.)
- close-output-file
- write-char
- write
- display
- error
- string-append
- system (non-standard) - excecutes shell code
- gensym


bootstrap.c currently recognises the following special forms:

- if
- set!
- define
- quote
- lambda
- begin
- cond, including =>
- let
- and
- or
- declare (non-standard) - ignored by the interpreter, the compiler will use them to aid compilation

bootstrap/lib.scm defines:

- number?
- list
- not
- append (varadic version)
- length
- reverse
- vector? (always false)
- range
- map
- filter
- foldl
- foldr
- accumulate
- all c...r functions
- memq
- memv

The following (non-standard) variable is availiable on startup:

 - args - command line arguments

Currently tested on Ubuntu only.
SHOULD work on all OSs currently (though untested) but I only plan to support unix.