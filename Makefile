scheme: bootstrap/bootstrap

bootstrap/bootstrap: cxrs.h util.o bootstrap/bootstrap.c bootstrap/bootstrap.h bootstrap/prims.c
	cd bootstrap && $(MAKE)

cxrs.h: cxrs.sh
	./cxrs.sh 4 > cxrs.h

util.o: util.c
	$(CC) -c util.c

util.c: util.h

.PHONY: clean
clean:
	-rm cxrs.h *.o
	cd bootstrap && $(MAKE) clean
