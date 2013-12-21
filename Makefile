scheme: Bootstrap

Bootstrap: bootstrap.c
	cc -o Bootstrap bootstrap.c

.PHONY : clean
clean :
	-rm Bootstrap