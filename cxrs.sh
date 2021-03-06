#!/bin/sh
# Writes to standard output all of the
# definitions such as 

  #define caar(x) car(car(x))
  #define cadr(x) car(cdr(x))
# ...
# up to $1 levels deep


cat <<INCLUDEGUARD
/* Generated by cxrs.sh */
#ifndef CXRS_H
#define CXRS_H

INCLUDEGUARD

#Outputs one definition given as and ds as inputs.
outputone(){
	macro="c"
	definition=""
	paren=

	for i in $@; do
		macro=$macro$i
		definition="${definition}c${i}r("
		paren=")$paren"
	done
	echo "#define ${macro}r(x) ${definition}x$paren"
}

#Outputs several definitions given strings of as and ds as inputs
output(){
	for i in $@; do
		outputone $(echo $i | sed 's/\(.\)/\1 /g')
	done
}

current="a d"
for i in $(seq 2 $1); do
	next=
	for x in $current; do
		next="$next a$x d$x"
	done
	output  $next
	current=$next
done

echo "#endif /* inclusion guard */"