#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "action.h"

void test_action();

int main(int argc, char **argv)
{
    printf("Running test_action()\n");
    test_action();

    return 0;
}

void test_action() {
    char *buffer;
    Action action;

    buffer = malloc(20);
    memcpy(buffer, "thisrandomteststring", 20);

    action = bs_new_action(HASH_CHUNK, buffer);

    assert(memcmp(action->data, buffer, 20) == 0);
    assert(action->type == HASH_CHUNK);

    free(buffer);

    assert(memcmp(action->data, "thisrandomteststring", 20) != 0);
}
