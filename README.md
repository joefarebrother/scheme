scheme
======

A scheme compiler


bootstrap.c is a bootstrap interpreter for scheme, intended for bootstraping compile.scm.
bootstrap-prims.c contains the primitive procedures for the bootstrapper
No other files are implemented yet.

bootstrap-prims.c currently defines:

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
- apply
- eval
- exit
- interaction-enviroment
- enviroment
- null-enviroment
- open-input-file
- read-char
- unread-char (non-standard)


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


To test, type at a terminal:

```shell
$ make
$ ./bootstrap
```

Currently tested on Ubuntu only.