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

#include <stdlib.h>
#include <string.h>
#include "chunk.h"

/**
 * Create a new chunk structure
 */
Chunk bs_new_chunk(int number, unsigned char *data, unsigned char *hash,
                   int chunk_size, int hash_length) {
    Chunk chunk = malloc(sizeof(struct bs_chunk_t));

    if (chunk != NULL) {
        chunk->number = number;
        chunk->chunk_size = chunk_size;
        chunk->data = malloc(chunk_size);
        if (chunk->data != NULL) {
            memcpy(chunk->data, data, chunk_size);
        }

        chunk->hash_length = hash_length;
        chunk->hash = malloc(hash_length);
        if (chunk->hash != NULL) {
            memcpy(chunk->hash, hash, hash_length);
        }
    }

    return chunk;
}

/**
 * Destroy and free up a chunk structure
 */
void bs_destroy_chunk(Chunk chunk) {
    memset(chunk->data, 0, chunk->chunk_size);
    memset(chunk->hash, 0, chunk->hash_length);
    chunk->number = -1;
    free(chunk->data);
    free(chunk->hash);
    free(chunk);
}
