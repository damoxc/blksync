#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"

void test_chunk(void);

int main(int argc, char **argv) {
    printf("Running test_chunk()\n");
    test_chunk();

    return 0;
}

void test_chunk(void) {
    unsigned char *buffer, *hash;
    Chunk chunk;

    buffer = malloc(4);
    hash = malloc(20);

    memcpy(buffer, "test", 4);
    memcpy(hash, "thistesthashforatest", 20);

    chunk = bs_new_chunk(1, buffer, hash, 4, 20);

    assert(memcmp(chunk->data, buffer, 4) == 0);
    assert(memcmp(chunk->hash, hash, 20) == 0);
    assert(chunk->number == 1);
    assert(chunk->chunk_size == 4);
    assert(chunk->hash_length == 20);

    free(buffer);
    free(hash);

    assert(memcmp(chunk->data, "test", 4) == 0);
    assert(memcmp(chunk->hash, "thistesthashforatest", 20) == 0);
}
