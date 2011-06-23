CC=gcc
CFLAGS=-Wall -Werror -O -g
LDFLAGS=-lrt -lgcrypt -lpthread
TARGETS=blksync testAction testChunk
CRYPTLIB=gcrypt
MUDFLAP=no

ifeq ($(CRYPTLIB),gcrypt)
CFLAGS+= -DUSE_GCRYPT
LDFLAGS+=-lgcrypt
endif

ifeq ($(CRYPTLIB),openssl)
CFLAGS+= -DUSE_OPENSSL
LDFLAGS+=-lssl
endif

ifeq ($(MUDFLAP),yes)
CFLAGS+= -fmudflapth
LDFLAGS+= -lmudflapth
endif

.PHONY: all clean

all: $(TARGETS)

blksync: action.o chunk.o worker.o writer.o

testAction: action.o

testChunk: chunk.o

clean:
	rm -f *.o $(TARGETS)
