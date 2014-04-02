scheme: bootstrap/bootstrap

bootstrap/bootstrap: cxrs.h
	cd bootstrap && $(MAKE)

cxrs.h: cxrs.sh
	./cxrs.sh 4 > cxrs.h

.PHONY: clean
clean:
	-rm cxrs.h 
	cd bootstrap && $(MAKE) clean
