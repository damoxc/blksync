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
#include <mqueue.h>
#include <sys/stat.h>

#ifdef USE_GCRYPT
#include <gcrypt.h>
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif

#include "action.h"
#include "chunk.h"
#include "common.h"
#include "worker.h"
#include "writer.h"

#define NUM_THREADS 1

/**
 * Main thead
 */
int main(int argc, char **argv) {
    FILE *bd_fp, *bf_fp, *h_fp;
    int bytes_read, hash_bytes_read, chunk_size = CHUNK_SIZE, i, chunk_count = 0, hash_length = SHA1_LENGTH;
    unsigned char *buffer, hash[hash_length];
    char *blockdev_name = "/dev/sysvg/bd-test";
    pthread_t *workers;
    pthread_attr_t pthread_custom_attr;
    params *p;
    Chunk chunk;
    mqd_t r_queue, w_queue;
    struct mq_attr qattrs;

#ifdef USE_GCRYPT

    if (!gcry_check_version(GCRYPT_VERSION)) {
        fputs ("libgcrypt version mismatch\n", stderr);
        exit(2);
    }
    gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED);
#endif

    qattrs.mq_msgsize = MSG_SIZE;
    qattrs.mq_maxmsg = 10;

    r_queue = mq_open("/bs-rq", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &qattrs);
    if (r_queue == -1) {
        perror("Unable to open read queue");
        return 1;
    }

    w_queue = mq_open("/bsync-wqueue", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &qattrs);
    if (w_queue == -1) {
        perror("Unable to open write queue");
        return 1;
    }

    //blockdev_name = "/dev/sysvg/vm_win7-32";

    // Open the block device
    bd_fp = fopen(blockdev_name, "r");
    if (bd_fp == NULL) {
        perror("Unable to open block device");
        return 1;
    }

    // Open the backup file
    bf_fp = bs_open_rw("/tmp/bd-test.img");
    if (bf_fp == NULL) {
        perror("Unable to open backup file");
        return 1;
    }

    // Open the hash file
    h_fp = bs_open_rw("/tmp/.bd-test.img.sha1");
    if (h_fp == NULL) {
        perror("Unable to open hash file");
        return 1;
    }

    // Initialize the variables
    buffer = (unsigned char *)malloc(chunk_size);

    // Initialize the threads
    workers = (pthread_t *)malloc(NUM_THREADS * sizeof(workers));
    pthread_attr_init(&pthread_custom_attr);
    p = (params *)malloc(sizeof(params) * NUM_THREADS);

    for (i = 0; i < NUM_THREADS; i++) {
        p[i].id = i;
        p[i].r_queue = r_queue;
        p[i].w_queue = w_queue;
        p[i].hash_length = SHA1_LENGTH;
        p[i].chunk_size = chunk_size;
        pthread_create(&workers[i], &pthread_custom_attr, bs_hash_chunk, (void *)(p+i));
    }

    // Beginning reading from the block device
    while (!feof(bd_fp)) {
        bytes_read = 0;

        // This loop handles partial reads
        while (bytes_read < chunk_size && !feof(bd_fp)) {
            bytes_read += fread(buffer + bytes_read, 1, chunk_size - bytes_read, bd_fp);
        }

        // Read in from our comparison hash, if we have one
        if (!feof(h_fp)) {
            hash_bytes_read = fread(hash, hash_length, 1, h_fp);
            if (hash_bytes_read == 0) {
                //hash = NULL;
            }
        }

        // Now we have a chunk in our buffer, stick it on the queue
        chunk = bs_new_chunk(chunk_count, buffer, hash, chunk_size, hash_length);
        printf("sender chunk-%d: %p\n", chunk_count, chunk);
        if (mq_send(r_queue, (char *)bs_new_action(HASH_CHUNK, chunk), MSG_SIZE, 5) == -1)
            perror("Unable to send to read queue");

        chunk_count++;

        break;
    }

    // End the threads
    //for (i = 0; i < NUM_THREADS; i++) {
    //    if (mq_send(r_queue, (char *)bs_new_action(END_THREAD, hash), MSG_SIZE, 5) == -1)
    //        perror("Unable to send to read queue");
    //}

    // Wait for the threads to exit
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(workers[i], NULL);
    }

    return 0;
};
