bootstrap: bootstrap.o prims.o ../util.o
	$(CC) bootstrap.o prims.o ../util.o -o bootstrap

bootstrap.o: bootstrap.h
	$(CC) -c bootstrap.c 

prims.o: bootstrap.h
	$(CC) -c prims.c

bootstrap.h: ../cxrs.h ../util.h

.PHONY: clean
clean:
	-rm *.o bootstrap