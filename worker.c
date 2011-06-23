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
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>

#include "action.h"
#include "chunk.h"
#include "common.h"
#include "worker.h"

/**
 * Hashing worker thread
 */
void *bs_hash_chunk(void *arg) {
    Chunk chunk;
    ssize_t msg_size;
    params *p = (params *)arg;
    Action action;
    unsigned char hash[p->hash_length];

    printf("Thread-%d starting life\n", p->id);

    action = malloc(sizeof(struct bs_action_t));

    while (1) {
        // Don't sit in a while loop if we can't get message, die!
        msg_size = mq_receive(p->r_queue, (char *)action, MSG_SIZE, NULL);
        //printf("receiver msg_size: %ld\n", msg_size);

        if (msg_size == -1 || action->type == END_THREAD) {
            if (msg_size == -1)
                perror("Unable to read from read-queue");
            break;
        }

        chunk = (Chunk)action->data;
        //printf("receiver chunk-%d: %p\n", chunk->number, chunk);

        assert(chunk != NULL);
        assert(chunk->data != NULL);
        assert(p != NULL);

        bs_sha1(hash, chunk->data, p->hash_length);

        bs_print_hash((unsigned char *)chunk->hash, p->hash_length);
        printf("\n");

        bs_print_hash((unsigned char *)hash, p->hash_length);
        printf("\n");

        bs_destroy_chunk(chunk);
        //printf("new_hash: %s\n", chunk->hash);

        //if (memcmp(chunk->hash, new_hash, SHA1_LENGTH) != 0) {
            // TODO: trigger write back to backup file
        //}
    }

    free(action);

    printf("Thread-%d ending life\n", p->id);
    return NULL;
}
