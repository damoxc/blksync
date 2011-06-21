blksync:
	gcc -lssl -lpthread -Wall -Werror -O -g -o blksync blksync.c -DUSE_OPENSSL

clean:
	rm -f blksync
