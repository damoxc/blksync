blksync:
	#gcc -lssl -Wall -Werror -O -g -o blksync blksync.c -DUSE_OPENSSL
	gcc -lgcrypt -Wall -Werror -O -g -o blksync blksync.c -DUSE_GCRYPT

clean:
	rm -f blksync
