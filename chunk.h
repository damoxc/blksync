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

typedef struct bs_chunk_t *Chunk;

struct bs_chunk_t {
    int            number;
    int            chunk_size;
    int            hash_length;
    unsigned char *data;
    unsigned char *hash;
};

/**
 * Create a new chunk structure
 */
Chunk bs_new_chunk(int number, unsigned char *data, unsigned char *hash,
                   int chunk_size, int hash_length);

/**
 * Destroy and free up a chunk structure
 */
void bs_destroy_chunk(Chunk chunk);
