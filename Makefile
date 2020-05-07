CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

orecc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): orecc.h

test: orecc
	./test.sh

clean:
	rm -f orecc *.o *~ tmp*

.PHONY: test clean
