/* 
 * A quick and dirty scheme interpreter for bootstrapping the 
 * compiler for the first time. Has pairs, lambdas, strings, 
 * integers, characters, symbols, and IO, all that is 
 * neccasary for the compiler. Includes a very simple 
 * reference-counting GC that will leak memory on circular 
 * structures, which is ok because only one (the global 
 * enviroment) is used.
 */
