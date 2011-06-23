#include <string.h>
#include <stdio.h>
#include "chunk.h"

void test_new_chunk() {
    unsigned char buffer[4], hash[20];
    Chunk chunk;

    memcpy(buffer, "test", 4);
    memcpy(hash, "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3", 20);

    chunk = bs_new_chunk(1, buffer, hash, 4, 20);
}

int main(void)
{
    printf("Running test_new_chunk()\n");
    test_new_chunk();

    return 0;
}
