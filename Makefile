scheme: Bootstrap

Bootstrap: bootstrap.c cxrs.h
	cc -o Bootstrap bootstrap.c

cxrs.h: cxrs.sh
	./cxrs.sh 4 > cxrs.h

.PHONY: clean debug
clean:
	-rm Bootstrap cxrs.h

debug:
	cc -g bootstrap.c -o Bootstrap