/***
 * blksync - synchronize chunks of data between files and/or block devices
 *
 * Copyright (C) 2011  Damien Churchill <damoxc@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define TRUE            1
#define FALSE           0
#define KB              1024
#define MB             (1024*KB)
#define CHUNK_SIZE     (4*MB)
#define NUM_THREADS     5
#define SHA1_LENGTH     20

#ifdef USE_OPENSSL
#include <openssl/sha.h>
void blksync_sha1(void *hash, const void *message, size_t length) {
	SHA1(message, length, hash);
}
#elif USE_GCRYPT
#include <gcrypt.h>
void blksync_sha1(void *hash, const void *message, size_t length) {
	gcry_md_hash_buffer(GCRY_MD_SHA1, hash, message, length);
}
#else
#error No cryptographic library specififed.
#endif

typedef struct chunk_t *Chunk;
typedef struct queue_t *Queue;

struct queue_t {
	Chunk head;
	Chunk tail;
};

struct chunk_t {
	unsigned char *data;
	Chunk next;
};

static Chunk new_chunk(unsigned char *data) {
	Chunk chunk = malloc(sizeof(struct chunk_t));
	if (chunk != NULL) {
		chunk->data = data;
		chunk->next = NULL;
	}
	return chunk;
};

static void destroy_chunk(Chunk chunk) {
	if (chunk != NULL) {
		Chunk next = chunk->next;
		chunk->data = '\0';
		chunk->next = NULL;
		free(chunk);
		destroy_chunk(next);
	}
};

Queue new_queue(void) {
	Queue queue = malloc(sizeof(struct queue_t));
	if (queue != NULL) {
		queue->head = NULL;
		queue->tail = NULL;
	}
	return queue;
}

void destroy_queue(Queue queue) {
	if (queue != NULL) {
		destroy_chunk(queue->head);
		queue->head = NULL;
		queue->tail = NULL;
		free(queue);
	}
}

int is_empty(Queue queue) {
	assert(queue != NULL);
	return (queue->head == NULL);
}

int enqueue(Queue queue, unsigned char *data) {
	assert(queue != NULL);

	int success = FALSE;
	Chunk chunk = new_chunk(data);

	if (chunk != NULL) {
		if (queue->tail != NULL) {
			assert(queue->tail->next == NULL);
			queue->tail->next = chunk;
		}

		if (queue->head == NULL) {
			queue->head = chunk;
		}

		queue->tail = chunk;
		success = TRUE;
	}

	return success;
}

unsigned char *dequeue(Queue queue) {
	assert(queue != NULL);
	assert(!is_empty(queue));

	Chunk first = queue->head;
	unsigned char *data = first->data;

	queue->head = first->next;

	if (queue->head == NULL) {
		assert(queue->tail == first);
		queue->tail = NULL;
	}

	free(first);

	return data;
}


/**
 * Using blksync_sha1:
 *
 * ssize_t length = 4;
 * char *message  = "test";
 * unsigned char buffer[SHA1_LENGTH];
 *
 * blksync_sha1(buffer, message, length);
 *
 * printf("%s\n", buffer);
 *
 * for (i = 0; i < SHA1_LENGTH; i++) {
 *     printf("%02x", buffer[i]);
 * }
 * printf("\n");
 */

int main(int argc, char **argv) {
	//Queue queue = new_queue();
	//pthread_t workers[NUM_THREADS];


	return 0;
};
