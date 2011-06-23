CC=gcc
CFLAGS=-Wall -Werror -O -g -DUSE_GCRYPT
LDFLAGS=-lssl -lrt -lgcrypt -lpthread
TARGETS=blksync testAction testChunk

.PHONY: all clean

all: $(TARGETS)

blksync: action.o chunk.o

testAction: action.o

testChunk: chunk.o

clean:
	rm -f *.o $(TARGETS)
