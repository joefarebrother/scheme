scheme
======

A scheme compiler


bootstrap.c is a bootstrap interpreter for scheme, intended for bootstraping compile.scm.
bootstrap-prims.c contains the primitive procedures for the bootstrapper
No other files are implemented yet.
bootstrap-prims.c currently defines:

char->integer
integer->char
number->string
string->number
symbol->string
string->symbol
+
-
*
quotient
remainder
=
<
>
cons
car
cdr
set-car!
set-cdr!
eq?

bootstrap.c currently recognises the following special forms:
if
set!
define*
quote
lambda*

*no sequential evaluation yet

To test, type at a terminal:
$ make
$ ./bootstrap

Currently tested on Ubuntu only.