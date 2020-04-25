CFLAGS=-std=c11 -g -static

orecc: orecc.c

test: orecc
	./test.sh

clean:
	rm -f orecc *.o *~ tmp*

.PHONY: test clean
