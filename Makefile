blksync:
	gcc -lgcrypt -lpthread -lrt -Wall -Werror -O -g -o blksync blksync.c -DUSE_GCRYPT

testChunk:
	gcc -Wall -Werror -O -g -o testChunk testChunk.c chunk.c
	./testChunk
	rm testChunk

clean:
	rm -f blksync
