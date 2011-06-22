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
#include <unistd.h>

#define TRUE            1
#define FALSE           0
#define KB              1024
#define MB             (1024*KB)
#define CHUNK_SIZE     (4*MB)
#define NUM_THREADS     4
#define SHA1_LENGTH     20

#ifdef USE_OPENSSL
#include <openssl/sha.h>
void blksync_sha1(void *hash, const void *message, size_t length) {
	SHA1(message, length, hash);
}
#elif USE_GCRYPT
#include <gcrypt.h>

GCRY_THREAD_OPTION_PTHREAD_IMPL;

void blksync_sha1(void *hash, const void *message, size_t length) {
	gcry_md_hash_buffer(GCRY_MD_SHA1, hash, message, length);
}
#else
#error No cryptographic library specififed.
#endif

pthread_mutex_t queue_mutex;

typedef struct chunk_t *Chunk;
typedef struct queue_t *Queue;

struct queue_t {
	Chunk head;
	Chunk tail;
};

struct chunk_t {
	unsigned char data[CHUNK_SIZE];
	unsigned char hash[SHA1_LENGTH];
	Chunk next;
};

typedef struct {
	Queue queue;
	int id;
} params;

static Chunk new_chunk(unsigned char *data, unsigned char *hash) {
	Chunk chunk = malloc(sizeof(struct chunk_t));
	if (chunk != NULL) {
		memcpy(&chunk->data, data, CHUNK_SIZE);
		memcpy(&chunk->hash, hash, SHA1_LENGTH);
		chunk->next = NULL;
	}
	return chunk;
};

static void destroy_chunk(Chunk chunk) {
	if (chunk != NULL) {
		Chunk next = chunk->next;
		memcpy(chunk->data, "\0", 1);
		memcpy(chunk->hash, "\0", 1);
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

int enqueue(Queue queue, unsigned char *data, unsigned char *hash) {
	assert(queue != NULL);

	int success = FALSE;
	Chunk chunk = new_chunk(data, hash);

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

Chunk dequeue(Queue queue) {
	assert(queue != NULL);
	assert(!is_empty(queue));

	Chunk chunk = queue->head;
	queue->head = chunk->next;

	if (queue->head == NULL) {
		assert(queue->tail == chunk);
		queue->tail = NULL;
	}

	return chunk;
}

FILE *blksync_open_rw(char *path) {
	FILE *fp;

	fp = fopen(path, "r+");
	if (fp == NULL && errno == ENOENT) {
		fp = fopen(path, "w+");
	}
	return fp;
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

void *hash_chunk(void *arg) {
	params *p = (params *)arg;
	unsigned char new_hash[SHA1_LENGTH];
	Chunk chunk;
	sleep(1);

	while (1) {
		pthread_mutex_lock(&queue_mutex);
		if (is_empty(p->queue)) {
			break;
		}
		chunk = dequeue(p->queue);
		pthread_mutex_unlock(&queue_mutex);

		blksync_sha1(new_hash, chunk->data, CHUNK_SIZE);

		if (memcmp(chunk->hash, new_hash, SHA1_LENGTH) != 0) {
			// TODO: trigger write back to backup file
		}

		pthread_mutex_lock(&queue_mutex);
		pthread_mutex_unlock(&queue_mutex);
	}
	return NULL;
}

int main(int argc, char **argv) {
	FILE *bd_fp, *bf_fp, *h_fp;
	int bytes_read, chunk_size = CHUNK_SIZE, i, first_run = 1;
	unsigned char buffer[chunk_size], hash[SHA1_LENGTH];
	char *blockdev_name = "/dev/sysvg/bd-test";
	Queue queue = new_queue();
	pthread_t *workers;
	pthread_attr_t pthread_custom_attr;
	params *p;

#ifdef USE_GCRYPT
	if (!gcry_check_version(GCRYPT_VERSION)) {
		fputs ("libgcrypt version mismatch\n", stderr);
		exit(2);
	}
	gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED);
#endif

	//blockdev_name = "/dev/sysvg/vm_win7-32";

	// Open the block device
	bd_fp = fopen(blockdev_name, "r");
	if (bd_fp == NULL) {
		perror("Unable to open block device");
		return 1;
	}

	// Open the backup file
	bf_fp = blksync_open_rw("/tmp/bd-test.img");
	if (bf_fp == NULL) {
		perror("Unable to open backup file");
		return 1;
	}

	// Open the hash file
	h_fp = blksync_open_rw("/tmp/.bd-test.img.sha1");
	if (h_fp == NULL) {
		perror("Unable to open hash file");
		return 1;
	}

	// Lock the queue mutex so the threads don't end prematurely
	pthread_mutex_lock(&queue_mutex);

	// Initialize the threads
	workers = (pthread_t *)malloc(NUM_THREADS * sizeof(workers));
	pthread_attr_init(&pthread_custom_attr);
	p = (params *)malloc(sizeof(params) * NUM_THREADS);

	for (i = 0; i < NUM_THREADS; i++) {
		p[i].id = i;
		p[i].queue = queue;
		pthread_create(&workers[i], &pthread_custom_attr, hash_chunk, (void *)(p+i));
	}

	// Beginning reading from the block device
	while (!feof(bd_fp)) {
		bytes_read = 0;

		// This loop handles partial reads
		while (bytes_read < chunk_size && !feof(bd_fp)) {
			bytes_read = fread(buffer + bytes_read, 1, chunk_size - bytes_read, bd_fp);
		}

		// Read in from our comparison hash, if we have one
		if (!feof(h_fp)) {
			bytes_read = fread(h_fp, SHA1_LENGTH, 1, h_fp);
		}

		// Open a lock on the queue
		if (!first_run)
			pthread_mutex_lock(&queue_mutex);

		// Now we have a chunk in our buffer, stick it on the queue
		enqueue(queue, buffer, hash);

		// Relase our lock on the queue
		pthread_mutex_unlock(&queue_mutex);

		first_run = 0;
	}

	// Wait for the threads to exit
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(workers[i], NULL);
	}

	return 0;
};
