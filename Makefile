scheme: bootstrap

bootstrap: bootstrap.c cxrs.h
	cc -o bootstrap bootstrap.c

cxrs.h: cxrs.sh
	./cxrs.sh 4 > cxrs.h

.PHONY: clean debug
clean:
	-rm bootstrap cxrs.h 

debug:
	cc -g bootstrap.c -o Bootstrap