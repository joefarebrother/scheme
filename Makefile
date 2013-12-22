scheme: Bootstrap

Bootstrap: bootstrap.c
	cc -o Bootstrap bootstrap.c

.PHONY: clean debug
clean:
	-rm Bootstrap

debug:
	cc -g bootstrap.c -o Bootstrap