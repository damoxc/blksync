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

#include <mqueue.h>

#ifndef BS_COMMON_H
#define BS_COMMON_H

#define TRUE            1
#define FALSE           0
#define KB              1024
#define MB             (1024*KB)
#define CHUNK_SIZE     (4*MB)
#define SHA1_LENGTH     20
#define MSG_SIZE        sizeof(struct bs_action_t)

// Thread parameters
typedef struct {
    mqd_t r_queue;
    mqd_t w_queue;
    int id;
    int hash_length;
    int chunk_size;
} params;

#ifdef USE_OPENSSL
#include <openssl/sha.h>
#elif USE_GCRYPT
#include <gcrypt.h>
#else
#error No cryptographic library specififed.
#endif

void bs_sha1(void *hash, const void *message, size_t length);

void bs_print_hash(unsigned char *hash, int length);

FILE *bs_open_rw(char *path);
#endif
