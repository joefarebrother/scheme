scheme: bootstrap

bootstrap: bootstrap.o bootstrap-prims.o
	cc bootstrap.o bootstrap-prims.o -o bootstrap 

bootstrap-prims.o: bootstrap-prims.c bootstrap.h cxrs.h
	cc -c bootstrap-prims.c 

bootstrap.o: bootstrap.c bootstrap.h cxrs.h
	cc -c bootstrap.c 

cxrs.h: cxrs.sh
	./cxrs.sh 4 > cxrs.h

.PHONY: clean
clean:
	-rm bootstrap cxrs.h *.o
